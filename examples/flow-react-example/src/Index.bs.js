// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE

import * as React from "react";
import * as Component1 from "./Component1.bs.js";
import * as Component2 from "./Component2.bs.js";
import * as ReactDOMRe from "reason-react/src/ReactDOMRe.js";

ReactDOMRe.renderToElementWithId(React.createElement(Component1.make, {
          message: "Hello!"
        }), "index1");

ReactDOMRe.renderToElementWithId(React.createElement(Component2.make, {
          greeting: "Hello!"
        }), "index2");

export {
  
}
/*  Not a pure module */
