/** 
 * @flow strict
 * @generated
 * @nolint
 */

const YY = {"NoOp": 0};

// $FlowExpectedError: Reason checked type sufficiently
import * as VariantsBS from './Variants.bs';

export type action = "NoOp" | {|tag: "AdjustAge", value: (number) => number|} | {|tag: "Booly", value: boolean|} | {|tag: "OptionalInt", value: ?number|} | {|tag: "Unity", value: void|} | {|tag: "OptionalBooly", value: ?boolean|} | {|tag: "OptionalBoolMapper", value: (?boolean) => ?boolean|};

export type optionalBoolMapper = {|+optionalBoolMapper: (?boolean) => ?boolean|};

export const actionToString: (action) => string = function _(Arg1) { const result = VariantsBS.actionToString((typeof(Arg1) === 'object' ? (Arg1.tag==="AdjustAge" ? [/* 0 */48, Arg1.value] :  Arg1.tag==="Booly" ? [/* 1 */49, Arg1.value] :  Arg1.tag==="OptionalInt" ? [/* 2 */50, (Arg1.value == null ? undefined : Arg1.value)] :  Arg1.tag==="Unity" ? [/* 3 */51, Arg1.value] :  Arg1.tag==="OptionalBooly" ? [/* 4 */52, (Arg1.value == null ? undefined : Arg1.value)] :  [/* 5 */53, function _(Arg11) { const result1 = Arg1.value(Arg11); return (result1 == null ? undefined : result1) }]) : YY[Arg1])); return result };

export const converter: (optionalBoolMapper) => optionalBoolMapper = function _(Arg1) { const result = VariantsBS.converter([function _(Arg11) { const result1 = Arg1.optionalBoolMapper(Arg11); return (result1 == null ? undefined : result1) }]); return {optionalBoolMapper:function _(Arg12) { const result2 = result[0]((Arg12 == null ? undefined : Arg12)); return result2 }} };
