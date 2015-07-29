# What is the V language?

The V language is a domain-specific language for capturing the operational values of a host computer's resources and for checking their validity against user-defined rules. The problem domain addressed by V is computer administration. In particular, V is well suited with helping an administrator to monitor a host's configuration, performance, and vulnerabilities.

## Nullipotent, declarative, interpreted

V is nullipotent, that is, its statements can be executed any number of times to produce the same result. The language is used only to observe the state of the host, never to alter its state.

V is a declarative language: it specifies what to do, but doesn't prescribe how to do it. Unlike imperative languages, V does not contain conditional branching statements or looping constructs. V is not a functional language and does not have subroutines, but V does have the concept of dependencies, which can be used to define the order of execution.

V is an interpreted language, rather than a compiled language, and thus requires no explicit pre-processing step. Language statements are inspected during the parsing stage and validated against the language's grammer and syntax.

## Namespaced objects, dynamic typing

V operates on objects, which describe host resource variables and user-defined rules. Host resource variables are described with resource objects, and user-defined rules are described with validation objects. Both types of objects use the same object definition syntax. Both types of objects belong to namespaces, where namespaces define defaults for the objects in their scope.

V is a dynamically typed language where the default type for all variables is a string. Whole numbers and real numbers are implicitly recognized and used—as integers and floating point values—in comparison operations.

## Extensible

V is extensible by design, and its interpreter can discover and use plugin modules, written by third-parties, without rebuilding the interpreter.

V is well suited to helping system administrators with monitoring computer systems using a declarative, interpreted, rule-based language that safely works without side effects.
