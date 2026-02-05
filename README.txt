VM execution pack (strict types + pass-by-reference params)

Files:
  - types.h      : Types enum (delete if your project already has Types)
  - vm.h / vm.cpp: POLIZ virtual machine
  - run.cpp      : example main that builds Syntaxer -> POLIZ -> runs VM

Important (minimal POLIZ/Syntaxer expectation):
  1) Addresses (variable offsets) must be emitted as POLIZ_Element::SYMBOL
     Literals must be emitted as POLIZ_Element::NUM
     Otherwise VM cannot distinguish literal '4' from address offset 4.

  2) For array element addresses:
     after computing baseOffset + indexOffset, emit POLIZ_Element::CALL_ARRAY
     so VM will treat it as Address, not as Value.

Quick build integration:
  Add vm.cpp to your build and compile run.cpp instead of old main.cpp.

If Types is already defined in your codebase:
  Remove types.h and include the existing header in vm.h instead.
