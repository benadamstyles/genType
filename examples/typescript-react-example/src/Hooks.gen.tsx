/* TypeScript file generated by genType. */
/* eslint-disable import/first */


// tslint:disable-next-line:no-var-requires
const Curry = require('bs-platform/lib/es6/curry.js');

// tslint:disable-next-line:no-var-requires
const HooksBS = require('./Hooks.bs');

// tslint:disable-next-line:interface-over-type-literal
export type vehicle = { readonly name: string };

// tslint:disable-next-line:interface-over-type-literal
export type callback<input,output> = (_1:input) => output;

// tslint:disable-next-line:interface-over-type-literal
export type testReactContext = (_1:number) => React.Context<number>;

export const $$default: (_1:{ readonly vehicle: vehicle }) => JSX.Element = function Hooks(Arg1: any) {
  const result = HooksBS.default({vehicle:[Arg1.vehicle.name]});
  return result
};

export default $$default;

export const anotherComponent: (_1:{ readonly vehicle: vehicle }) => JSX.Element = function Hooks_anotherComponent(Arg1: any) {
  const result = HooksBS.anotherComponent({vehicle:[Arg1.vehicle.name]});
  return result
};

export const Inner_make: (_1:{ readonly vehicle: vehicle }) => JSX.Element = function Hooks_Inner(Arg1: any) {
  const result = HooksBS.Inner[0]({vehicle:[Arg1.vehicle.name]});
  return result
};

export const Inner_anotherComponent: (_1:{ readonly vehicle: vehicle }) => JSX.Element = function Hooks_Inner_anotherComponent(Arg1: any) {
  const result = HooksBS.Inner[1]({vehicle:[Arg1.vehicle.name]});
  return result
};

export const Inner_Inner2_make: (_1:{ readonly vehicle: vehicle }) => JSX.Element = function Hooks_Inner_Inner2(Arg1: any) {
  const result = HooksBS.Inner[2][0]({vehicle:[Arg1.vehicle.name]});
  return result
};

export const Inner_Inner2_anotherComponent: (_1:{ readonly vehicle: vehicle }) => JSX.Element = function Hooks_Inner_Inner2_anotherComponent(Arg1: any) {
  const result = HooksBS.Inner[2][1]({vehicle:[Arg1.vehicle.name]});
  return result
};

export const makeWithRef: (_1:{ readonly vehicle: vehicle }, _2:(null | undefined | any)) => JSX.Element = function (Arg1: any, Arg2: any) {
  const result = Curry._2(HooksBS.makeWithRef, {vehicle:[Arg1.vehicle.name]}, Arg2);
  return result
};

export const testForwardRef: (_1:{ readonly vehicle: vehicle }) => JSX.Element = function Hooks_testForwardRef(Arg1: any) {
  const result = HooksBS.testForwardRef({vehicle:[Arg1.vehicle.name]});
  return result
};
