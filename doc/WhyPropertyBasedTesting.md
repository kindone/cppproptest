# Why Property-based testing?

#### Generalization and abstraction

Property-based testing, or PBT in short, lets you write a test using abstract idea, instead of some dummy examples or contrived scenarios that are easy to break over time and hard to interpret. You can write tests using specification or requirements - which are essentially expressed as combination of input domain and expected behavior (=properties) of tested component.

#### Clear separation of variants and invariants

Describing a test using input domain and expected behavior means that we have clear cut between variants and invariants.
We often mix variants and invariants in our tests in typical *example-based tests*. This is one of the reasons why those tests become hard to maintain and difficult to read over time. In property-based tests, however, we clearly separate invariants as properties and variants as input domain. Thus tests become more readable and gets easier to maintain, as they carry test writers' intention better.

#### Convenience and versatility

Property-based testing often provides with the convenient out-of-box generators (sometimes called *strategies*) and combinators (that lets you make new generators out of existing ones), and lets you to effortlessly define the input domain for your tests. You can specify your input domains using various notions - range, constraint, union, transformation, composition, and dependency, just to name a few.

#### Reusability and Scalability

As it defines input domain with generators and combinators, Property-based testing lets you define and reuse existing input domain and properties to build new input domain and properties with ease. These new ones then can be reused as building blocks for next tests and so on. Reusable nature of property-based testing lets you write tests with scale. Complex combinations of components can be readily tested if simpler pieces are already available by tests written previously.

&nbsp;

