# Stateful testing

While property-based testing suits well with functions and stateless objects, it's possible to test various state changes with ease. The idea of stateful testing with `cppproptest` is to define and utilize generators for state changes.

* Define `action`s that each represents unit of state change - e.g. calling specific method with arguments
* Define a generator for `action` type that can build a seqeunce of actions and provide arguments to the selected actions
* Run the action sequences generated as usual
