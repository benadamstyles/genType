open GenTypeCommon;

type t =
  | ArrayC(t)
  | CircularC(string, t)
  | EnumC(enum)
  | FunctionC(list(groupedArgConverter), t)
  | IdentC
  | NullableC(t)
  | ObjectC(fieldsC)
  | OptionC(t)
  | RecordC(fieldsC)
and groupedArgConverter =
  | ArgConverter(label, t)
  | GroupConverter(list((string, t)))
and fieldsC = list((string, t));

let rec toString = converter =>
  switch (converter) {
  | ArrayC(c) => "array(" ++ toString(c) ++ ")"

  | CircularC(s, c) => "circular(" ++ s ++ " " ++ toString(c) ++ ")"

  | EnumC({cases, _}) =>
    "enum("
    ++ (cases |> List.map(case => case.labelJS) |> String.concat(", "))
    ++ ")"

  | FunctionC(groupedArgConverters, c) =>
    let labelToString = label =>
      switch (label) {
      | Nolabel => "_"
      | Label(_) => "~l"
      | OptLabel(l) => "~?" ++ l
      };
    "fn("
    ++ (
      groupedArgConverters
      |> List.map(groupedArgConverter =>
           switch (groupedArgConverter) {
           | ArgConverter(label, conv) =>
             "(" ++ labelToString(label) ++ ":" ++ toString(conv) ++ ")"
           | GroupConverter(groupConverters) =>
             "{|"
             ++ (
               groupConverters
               |> List.map(((s, argConverter)) =>
                    s ++ ":" ++ toString(argConverter)
                  )
               |> String.concat(", ")
             )
             ++ "|}"
           }
         )
      |> String.concat(", ")
    )
    ++ " -> "
    ++ toString(c)
    ++ ")";

  | IdentC => "id"

  | NullableC(c) => "nullable(" ++ toString(c) ++ ")"

  | ObjectC(fieldsC)
  | RecordC(fieldsC) =>
    let dot =
      switch (converter) {
      | ObjectC(_) => ". "
      | _ => ""
      };
    "{"
    ++ dot
    ++ (
      fieldsC
      |> List.map(((lbl, c)) => lbl ++ ":" ++ (c |> toString))
      |> String.concat(", ")
    )
    ++ "}";

  | OptionC(c) => "option(" ++ toString(c) ++ ")"
  };

let typToConverter = (~language, ~exportTypeMap, ~typesFromOtherFiles, typ) => {
  let circular = ref("");
  let rec visit = (~visited: StringSet.t, typ) =>
    switch (typ) {
    | Array(t, _) =>
      let converter = t |> visit(~visited);
      ArrayC(converter);

    | Enum(cases) => EnumC(cases)

    | Function({argTypes, retType, _}) =>
      let argConverters =
        argTypes |> List.map(typToGroupedArgConverter(~visited));
      let retConverter = retType |> visit(~visited);
      FunctionC(argConverters, retConverter);

    | GroupOfLabeledArgs(_) => IdentC

    | Ident(s, typeArguments) =>
      if (visited |> StringSet.mem(s)) {
        circular := s;
        IdentC;
      } else {
        try (
          {
            let visited = visited |> StringSet.add(s);
            let (typeVars, t) =
              try (exportTypeMap |> StringMap.find(s)) {
              | Not_found => typesFromOtherFiles |> StringMap.find(s)
              };
            let pairs =
              try (List.combine(typeVars, typeArguments)) {
              | Invalid_argument(_) => []
              };

            let f = typeVar =>
              switch (
                pairs |> List.find(((typeVar1, _)) => typeVar == typeVar1)
              ) {
              | (_, typeArgument) => Some(typeArgument)
              | exception Not_found => None
              };
            t |> TypeVars.substitute(~f) |> visit(~visited);
          }
        ) {
        | Not_found => IdentC
        };
      }
    | Nullable(t) =>
      let converter = t |> visit(~visited);
      NullableC(converter);

    | Object(fields) =>
      ObjectC(
        fields
        |> List.map(((lbl, optionalness, t)) =>
             (
               lbl,
               (optionalness == Mandatory ? t : Option(t)) |> visit(~visited),
             )
           ),
      )

    | Option(t) => OptionC(t |> visit(~visited))

    | Record(fields) =>
      RecordC(
        fields
        |> List.map(((lbl, optionalness, t)) =>
             (
               lbl,
               (optionalness == Mandatory ? t : Option(t)) |> visit(~visited),
             )
           ),
      )

    | TypeVar(_) => IdentC
    }
  and typToGroupedArgConverter = (~visited, typ) =>
    switch (typ) {
    | GroupOfLabeledArgs(fields) =>
      GroupConverter(
        fields
        |> List.map(((s, _optionalness, t)) => (s, t |> visit(~visited))),
      )
    | _ => ArgConverter(Nolabel, typ |> visit(~visited))
    };

  let converter = typ |> visit(~visited=StringSet.empty);
  if (Debug.converter) {
    logItem(
      "Converter typ:%s converter:%s\n",
      typ |> EmitTyp.typToString(~language),
      converter |> toString,
    );
  };
  circular^ != "" ? CircularC(circular^, converter) : converter;
};

let rec converterIsIdentity = (~toJS, converter) =>
  switch (converter) {
  | ArrayC(c) => c |> converterIsIdentity(~toJS)

  | CircularC(_, c) => c |> converterIsIdentity(~toJS)

  | EnumC(_) => false

  | FunctionC(groupedArgConverters, resultConverter) =>
    resultConverter
    |> converterIsIdentity(~toJS)
    && groupedArgConverters
    |> List.for_all(groupedArgConverter =>
         switch (groupedArgConverter) {
         | ArgConverter(label, argConverter) =>
           label == Nolabel
           && argConverter
           |> converterIsIdentity(~toJS=!toJS)
         | GroupConverter(_) => false
         }
       )

  | IdentC => true

  | NullableC(c) => c |> converterIsIdentity(~toJS)

  | ObjectC(fieldsC) =>
    fieldsC
    |> List.for_all(((_, c)) =>
         switch (c) {
         | OptionC(c1) => c1 |> converterIsIdentity(~toJS)
         | _ => c |> converterIsIdentity(~toJS)
         }
       )

  | OptionC(c) =>
    if (toJS) {
      c |> converterIsIdentity(~toJS);
    } else {
      false;
    }

  | RecordC(_) => false
  };

let rec apply = (~converter, ~enumTables, ~toJS, value) =>
  switch (converter) {
  | _ when converter |> converterIsIdentity(~toJS) => value

  | ArrayC(c) =>
    value
    ++ ".map(function _element(x) { return "
    ++ ("x" |> apply(~converter=c, ~enumTables, ~toJS))
    ++ "})"

  | CircularC(s, c) =>
    "\n/* WARNING: circular type "
    ++ s
    ++ ". Only shallow converter applied. */\n  "
    ++ value
    |> apply(~converter=c, ~enumTables, ~toJS)

  | EnumC({cases: [case], _}) =>
    toJS ?
      case.labelJS |> EmitText.quotes : case.label |> Runtime.emitVariantLabel

  | EnumC(enum) =>
    let table = toJS ? enum.toJS : enum.toRE;
    Hashtbl.replace(enumTables, table, (enum, toJS));
    table ++ EmitText.array([value]);

  | FunctionC(groupedArgConverters, resultConverter) =>
    let resultVar = "result";
    let mkReturn = x =>
      "const "
      ++ resultVar
      ++ " = "
      ++ x
      ++ "; return "
      ++ apply(~converter=resultConverter, ~enumTables, ~toJS, resultVar);
    let convertedArgs = {
      let convertArg = (i, groupedArgConverter) =>
        switch (groupedArgConverter) {
        | ArgConverter(lbl, argConverter) =>
          let varName =
            switch (lbl) {
            | Nolabel => EmitText.argi(i + 1)
            | Label(l)
            | OptLabel(l) => EmitText.arg(l)
            };
          let notToJS = !toJS;
          (
            varName,
            varName
            |> apply(~converter=argConverter, ~enumTables, ~toJS=notToJS),
          );
        | GroupConverter(groupConverters) =>
          let notToJS = !toJS;
          if (toJS) {
            let varName = EmitText.argi(i + 1);
            (
              varName,
              groupConverters
              |> List.map(((s, argConverter)) =>
                   varName
                   ++ "."
                   ++ s
                   |> apply(
                        ~converter=argConverter,
                        ~enumTables,
                        ~toJS=notToJS,
                      )
                 )
              |> String.concat(", "),
            );
          } else {
            let varNames =
              groupConverters
              |> List.map(((s, _argConverter)) => EmitText.arg(s))
              |> String.concat(", ");
            let fieldValues =
              groupConverters
              |> List.map(((s, argConverter)) =>
                   s
                   ++ ":"
                   ++ (
                     EmitText.arg(s)
                     |> apply(
                          ~converter=argConverter,
                          ~enumTables,
                          ~toJS=notToJS,
                        )
                   )
                 )
              |> String.concat(", ");
            (varNames, "{" ++ fieldValues ++ "}");
          };
        };
      groupedArgConverters |> List.mapi(convertArg);
    };
    let mkBody = args => value |> EmitText.funCall(~args) |> mkReturn;
    EmitText.funDef(~args=convertedArgs, ~mkBody, "");

  | IdentC => value

  | NullableC(c) =>
    EmitText.parens([
      value
      ++ " == null ? "
      ++ value
      ++ " : "
      ++ (value |> apply(~converter=c, ~enumTables, ~toJS)),
    ])

  | ObjectC(fieldsC) =>
    let simplifyFieldConverted = fieldConverter =>
      switch (fieldConverter) {
      | OptionC(converter1) when converter1 |> converterIsIdentity(~toJS) =>
        IdentC
      | _ => fieldConverter
      };
    let fieldValues =
      fieldsC
      |> List.map(((lbl, fieldConverter)) =>
           lbl
           ++ ":"
           ++ (
             value
             ++ "."
             ++ lbl
             |> apply(
                  ~converter=fieldConverter |> simplifyFieldConverted,
                  ~enumTables,
                  ~toJS,
                )
           )
         )
      |> String.concat(", ");
    "{" ++ fieldValues ++ "}";

  | OptionC(c) =>
    if (toJS) {
      EmitText.parens([
        value
        ++ " == null ? "
        ++ value
        ++ " : "
        ++ (value |> apply(~converter=c, ~toJS, ~enumTables)),
      ]);
    } else {
      EmitText.parens([
        value
        ++ " == null ? undefined : "
        ++ (value |> apply(~converter=c, ~enumTables, ~toJS)),
      ]);
    }

  | RecordC(fieldsC) =>
    let simplifyFieldConverted = fieldConverter =>
      switch (fieldConverter) {
      | OptionC(converter1) when converter1 |> converterIsIdentity(~toJS) =>
        IdentC
      | _ => fieldConverter
      };
    if (toJS) {
      let fieldValues =
        fieldsC
        |> List.mapi((i, (lbl, fieldConverter)) =>
             lbl
             ++ ":"
             ++ (
               value
               ++ "["
               ++ string_of_int(i)
               ++ "]"
               |> apply(
                    ~converter=fieldConverter |> simplifyFieldConverted,
                    ~enumTables,
                    ~toJS,
                  )
             )
           )
        |> String.concat(", ");
      "{" ++ fieldValues ++ "}";
    } else {
      let fieldValues =
        fieldsC
        |> List.map(((lbl, fieldConverter)) =>
             value
             ++ "."
             ++ lbl
             |> apply(
                  ~converter=fieldConverter |> simplifyFieldConverted,
                  ~enumTables,
                  ~toJS,
                )
           )
        |> String.concat(", ");
      "[" ++ fieldValues ++ "]";
    };
  };

let toJS = (~converter, ~enumTables, value) =>
  value |> apply(~converter, ~enumTables, ~toJS=true);

let toReason = (~converter, ~enumTables, value) =>
  value |> apply(~converter, ~enumTables, ~toJS=false);