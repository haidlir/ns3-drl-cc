.. include:: replace.txt
.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

.. _Coding style:

Coding style
------------

When writing code to be contributed to the |ns3| open source project, we
ask that you follow the coding standards, guidelines, and recommendations
found below.

Clang-format
************

The |ns3| project uses `clang-format <https://clang.llvm.org/docs/ClangFormat.html>`_
to define and enforce the C++ coding style. Clang-format can be easily integrated
with modern IDEs or run manually on the command-line.

Clang-format installation
=========================

Clang-format can be installed using your OS's package manager. Please note that you
should install one of the supported versions of clang-format, which are listed in the
following section.

Supported versions of clang-format
==================================

Since each new major version of clang-format can add or modify properties,
newer versions of clang-format might produce different outputs compared to
previous versions.

The following list contains the set of clang-format versions that are verified
to produce consistent output among themselves.

* clang-format-14
* clang-format-15
* clang-format-16

Integration with IDEs
=====================

Clang-format can be integrated with modern IDEs (e.g., VS Code) that are able to
read the ``.clang-format`` file and automatically format the code on save or on typing.

Please refer to the documentation of your IDE for more information.
Some examples of IDE integration are provided in
`clang-format documentation <https://clang.llvm.org/docs/ClangFormat.html>`_

As an example, VS Code can be configured to automatically format code on save, on paste
and on type by enabling the following settings:

.. sourcecode:: json

  {
    "editor.formatOnSave": true,
    "editor.formatOnPaste": true,
    "editor.formatOnType": true,
  }

Clang-format usage in the terminal
==================================

In addition to IDE support, clang-format can be manually run on the terminal.

To format a file in-place, run the following command:

.. sourcecode:: console

  clang-format -i $FILE

To check that a file is well formatted, run the following command on the terminal.
If the file is not well formatted, clang-format indicates the lines that need to be
formatted.

.. sourcecode:: console

  clang-format --dry-run $FILE

Clang-format Git integration
============================

Clang-format can be integrated with Git to reformat existing Git patches, such as
pending merge requests on the GitLab repository. The full documentation is available on
`clang-format Git integration <https://clang.llvm.org/docs/ClangFormat.html#git-integration>`_

In addition to Git patches,
`clang-format-diff <https://clang.llvm.org/docs/ClangFormat.html#script-for-patch-reformatting>`_
can also be used to reformat existing patches produced with the ``diff`` tool.

Disable formatting in specific files or lines
=============================================

To disable formatting in specific lines, surround them with the following
C++ comments [example adopted from
`official clang-format documentation <https://clang.llvm.org/docs/ClangFormat.html>`_]:

.. sourcecode:: cpp

  // clang-format off
  ...
  // clang-format on

To exclude an entire file from being formatted, surround the whole file with the
special comments.

check-style-clang-format.py
***************************

To facilitate checking and fixing source code files according to the |ns3| coding style,
|ns3| maintains the ``check-style-clang-format.py`` Python script (located in ``utils/``).
This script is a wrapper to clang-format and provides useful options to check and fix
source code files. Additionally, it checks and fixes trailing whitespace and tabs in text
files.

We recommend running this script over your newly introduced C++ files prior to submission
as a Merge Request.

The script has multiple modes of operation. By default, the script checks if
source code files are well formatted and text files do not have trailing whitespace
nor tabs. The process returns a zero exit code if all files adhere to these rules.
If there are files that do not comply with the rules, the process returns a non-zero
exit code and lists the respective files. This mode is useful for developers editing
their code and for the GitLab CI/CD pipeline to check if the codebase is well formatted.
All checks are enabled by default. Users can disable specific checks using the corresponding
flags: ``--no-formatting``, ``--no-whitespace`` and ``--no-tabs``.

In addition to checking the files, the script can automatically fix detected issues in-place.
This mode is enabled by adding the ``--fix`` flag.

The formatting and tabs checks respect clang-format guards, which mark code blocks
that should not be checked. Trailing whitespace is always checked regardless of
clang-format guards.

The complete API of the ``check-style-clang-format.py`` script can be obtained with the
following command:

.. sourcecode:: console

  ./utils/check-style-clang-format.py --help

For quick-reference, the most used commands are listed below:

.. sourcecode:: console

  # Entire codebase (using paths relative to the ns-3 main directory)
  ./utils/check-style-clang-format.py [--fix] [--no-formatting] [--no-whitespace] [--no-tabs] .

  # Entire codebase (using absolute paths)
  /path/to/utils/check-style-clang-format.py [--fix] [--no-formatting] [--no-whitespace] [--no-tabs] /path/to/ns3

  # Specific directory
  /path/to/utils/check-style-clang-format.py [--fix] [--no-formatting] [--no-whitespace] [--no-tabs] absolute_or_relative/path/to/directory

  # Individual file
  /path/to/utils/check-style-clang-format.py [--fix] [--no-formatting] [--no-whitespace] [--no-tabs] absolute_or_relative/path/to/file


Clang-tidy
**********

The |ns3| project uses `clang-tidy <https://clang.llvm.org/extra/clang-tidy/>`_
to statically analyze (lint) C++ code and help developers write better code.
Clang-tidy can be easily integrated with modern IDEs or run manually on the command-line.

The list of clang-tidy checks currently enabled in the |ns3| project is saved on the
``.clang-tidy`` file. The full list of clang-tidy checks and their detailed explanations
can be seen in `Clang-tidy checks <https://clang.llvm.org/extra/clang-tidy/checks/list.html>`_.

Clang-tidy installation
=======================

Clang-format can be installed using your OS's package manager. Please note that you
should install one of the supported versions of clang-format, which are listed in the
following section.

Minimum clang-tidy version
==========================

Since clang-tidy is a linter that analyzes code and outputs errors found during
the analysis, developers can use different versions of clang-tidy on the workflow.
Newer versions of clang-tidy might produce better results than older versions.
Therefore, it is recommended to use the latest version available.

To ensure consistency among developers, |ns3| defines a minimum version of clang-tidy,
whose warnings must not be ignored. Therefore, developers should, at least, scan their
code with the minimum version of clang-tidy.

The minimum version is clang-tidy-14.

Integration with IDEs
=====================

Clang-tidy automatically integrates with modern IDEs (e.g., VS Code) that read the
``.clang-tidy`` file and automatically checks the code of the currently open file.

Please refer to the documentation of your IDE for more information.
Some examples of IDE integration are provided in
`clang-tidy documentation <https://clang.llvm.org/extra/clang-tidy/Integrations.html>`_

Clang-tidy usage
================

In order to use clang-tidy, |ns3| must first be configured with the flag
``--enable-clang-tidy``. To configure |ns3| with tests, examples and clang-tidy enabled,
run the following command:

.. sourcecode:: console

  ./ns3 configure --enable-examples --enable-tests --enable-clang-tidy

Due to the integration of clang-tidy with CMake, clang-tidy can be run while building
|ns3|. In this way, clang-tidy errors will be shown alongside build errors on the terminal.
To build |ns3| and run clang-tidy, run the the following command:

.. sourcecode:: console

  ./ns3 build

To run clang-tidy without building |ns3|, use the following commands on the ns-3
main directory:

.. sourcecode:: console

  # Analyze (and fix) a single file with clang-tidy
  clang-tidy -p cmake-cache/ [--fix] [--format-style=file] [--quiet] $FILE

  # Analyze (and fix) multiple files in parallel
  run-clang-tidy -p cmake-cache/ [-fix] [-format] [-quiet] $FILE1 $FILE2 ...

  # Analyze (and fix) the entire ns-3 codebase in parallel
  run-clang-tidy -p cmake-cache/ [-fix] [-format] [-quiet]

When running clang-tidy, please note that:

- Clang-tidy only analyzes implementation files (i.e., ``*.cc`` files).
  Header files are analyzed when they are included by implementation files, with the
  ``#include "..."`` directive.

- Not all clang-tidy checks provide automatic fixes. For those cases, a manual fix
  must be made by the developer.

- Enabling clang-tidy will add time to the build process (in the order of minutes).

Disable clang-tidy analysis in specific lines
=============================================

To disable clang-tidy analysis of a particular rule in a specific function,
specific clang-tidy comments have to be added to the corresponding function.
Please refer to the `official clang-tidy documentation <https://clang.llvm.org/extra/clang-tidy/#suppressing-undesired-diagnostics>`_
for more information.

To disable ``modernize-use-override`` checking on ``func()`` only, use one of the
following two "special comment" syntaxes:

.. sourcecode:: cpp

  //
  // Syntax 1: Comment above the function
  //

  // NOLINTNEXTLINE(modernize-use-override)
  void func();

  //
  // Syntax 2: Trailing comment
  //

  void func(); // NOLINT(modernize-use-override)

To disable a specific clang-tidy check on a block of code, for instance the
``modernize-use-override`` check, surround the code block with the following
"special comments":

.. sourcecode:: cpp

  // NOLINTBEGIN(modernize-use-override)
  void func1();
  void func2();
  // NOLINTEND(modernize-use-override)


To disable all clang-tidy checks on a block of code, surround it with the
following "special comments":

.. sourcecode:: cpp

  // NOLINTBEGIN
  void func1();
  void func2();
  // NOLINTEND

To exclude an entire file from being checked, surround the whole file with the
"special comments".

Source code formatting
**********************

The |ns3| coding style was changed between the ns-3.36 and ns-3.37 release.
Prior to ns-3.37, |ns3| used a base GNU coding style. Since ns-3.37, |ns3| changed the
base coding style to what is known in the industry as Allman-style braces,
with four-space indentation. In clang-format, this is configured by selecting the
``Microsoft`` base style. The following examples illustrate the style.

Indentation
===========

Indent code with 4 spaces. When breaking statements into multiple lines, indent the
following lines with 4 spaces.

.. sourcecode:: cpp

  void
  Func()
  {
      int x = 1;
  }

Indent constructor's initialization list with 4 spaces.

.. sourcecode:: cpp

  MyClass::MyClass(int x, int y)
      : m_x (x),
        m_y (y)
  {
  }

Do not use tabs in source code. Always use spaces for indentation and alignment.

Line endings
============

Files use LF (``\n``) line endings.

Column limit
============

Code lines should not extend past 100 characters. This allows reading code in wide-screen
monitors without having to scroll horizontally, while also allowing editing two files
side-by-side.

Braces style
============

Braces should be formatted according to the Allman style.
Braces are always on a new line and aligned with the start of the corresponding block.
The main body is indented with 4 spaces.

Always surround conditional or loop blocks (e.g., ``if``, ``for``, ``while``)
with braces, and always add a space before the condition's opening parentheses.

.. sourcecode:: cpp

  void Foo()
  {
      if (condition)
      {
          // do stuff here
      }
      else
      {
          // do other stuff here
      }

      for (int i = 0; i < 100; i++)
      {
          // do loop
      }

      while (condition)
      {
          // do while
      }

      do
      {
          // do stuff
      } while (condition);
  }

Spacing
=======

To increase readability, functions, classes and namespaces are separated by one
new line. This spacing is optional when declaring variables or functions.
Declare one variable per line. Do not mix multiple statements on the same line.

Do not add a space between the function name and the opening parentheses.
This rule applies to both function (and method) declarations and invocations.

.. sourcecode:: cpp

  void Func(const T&);  // OK
  void Func (const T&); // Not OK

Trailing whitespace
===================

Source code and text files must not have trailing whitespace.

Code alignment
==============

To improve code readability, trailing comments should be aligned.

.. sourcecode:: cpp

  int varOne;    // Variable one
  double varTwo; // Variable two

The trailing ``\`` character of macros should be aligned to the far right
(equal to the column limit). This increases the readability of the macro's body,
without forcing unnecessary whitespace diffs on surrounding lines when only one
line is changed.

.. sourcecode:: cpp

  #define MY_MACRO(msg)                      \
      do                                     \
      {                                      \
          std::cout << msg << std::endl;     \
      } while (false);


Class members
=============

Definition blocks within a class should be organized in descending order of
public exposure, that is: ``static`` > ``public`` > ``protected`` > ``private``.
Separate each block with a new line.

.. sourcecode:: cpp

  class MyClass
  {
  public:
      static int m_counter = 0;

      MyClass(int x, int y);

  private:
      int x;
      int y;
  };

Function arguments bin packing
==============================

Function arguments should be declared in the same line as the function declaration.
If the arguments list does not fit the maximum column width, declare each one on a
separate line and align them vertically.

.. sourcecode:: cpp

  void ShortFunction(int x, int y);

  void VeryLongFunctionWithLongArgumentList(int x,
                                            int y,
                                            int z);


The constructor initializers are always declared one per line, with a trailing comma:

.. sourcecode:: cpp

  void
  MyClass::MyClass(int x, int y)
      : m_x(x),
        m_y(y)
  {
  }

Function return types
=====================

In function declarations, return types are declared on the same line.
In function implementations, return types are declared on a separate line.


.. sourcecode:: cpp

  // Function declaration
  void Func(int x, int y);

  // Function implementation
  void
  Func(int x, int y)
  {
      // (...)
  }

Templates
=========

Template definitions are always declared in a separate line from the main function
declaration:

.. sourcecode:: cpp

  template <class T>
  void Func(T t);

Naming
======

Name encoding
#############

Function, method, and type names should follow the
`CamelCase <https://en.wikipedia.org/wiki/CamelCase>`_ convention: words are
joined without spaces and are capitalized. For example, "my computer" is
transformed into ``MyComputer``.  Do not use all capital letters such as
``MAC`` or ``PHY``, but choose instead ``Mac`` or ``Phy``. Do not use all
capital letters, even for acronyms such as ``EDCA``; use ``Edca`` instead.
This applies also to two-letter acronyms, such as ``IP`` (which becomes
``Ip``). The goal of the CamelCase convention is to ensure that the words
which make up a name can be separated by the eye: the initial Caps
fills that role. Use PascalCasing (CamelCase with first letter capitalized)
for function, property, event, and class names.

Variable names should follow a slight variation on the base CamelCase
convention: camelBack. For example, the variable ``user name`` would be named
``userName``. This variation on the basic naming pattern is used to allow a
reader to distinguish a variable name from its type. For example,
``UserName userName`` would be used to declare a variable named
``userName`` of type ``UserName``.

Global variables should be prefixed with a ``g_`` and member variables
(including static member variables) should be prefixed with a ``m_``. The goal
of that prefix is to give a reader a sense of where a variable of a given
name is declared to allow the reader to locate the variable declaration and
infer the variable type from that declaration. Defined types will start
with an upper case letter, consist of upper and lower case letters, and may
optionally end with a ``_t``. Defined constants (such as static const class
members, or enum constants) will be all uppercase letters or numeric digits,
with an underscore character separating words. Otherwise, the underscore
character should not be used in a variable name. For example, you could
declare in your class header ``my-class.h``:

.. sourcecode:: cpp

  typedef int NewTypeOfInt_t;
  constexpr uint8_t PORT_NUMBER = 17;

  class MyClass
  {
      void MyMethod(int aVar);
      int m_aVar;
      static int m_anotherVar;
  };

and implement in your class file ``my-class.cc``:

.. sourcecode:: cpp

  int MyClass::m_anotherVar = 10;
  static int g_aStaticVar = 100;
  int g_aGlobalVar = 1000;

  void
  MyClass::MyMethod(int aVar)
  {
      m_aVar = aVar;
  }

As an exception to the above, the members of structures do not need to be
prefixed with an ``m_``.

Finally, do not use
`Hungarian notation <https://en.wikipedia.org/wiki/Hungarian_notation>`_, and
do not prefix enums, classes, or delegates with any letter.

Choosing names
##############

Variable, function, method, and type names should be based on the English
language, American spelling. Furthermore, always try to choose descriptive
names for them. Types are often english names such as: Packet, Buffer, Mac,
or Phy. Functions and methods are often named based on verbs and adjectives:
``GetX``, ``DoDispose``, ``ClearArray``, etc.

A long descriptive name which requires a lot of typing is always better than
a short name which is hard to decipher. Do not use abbreviations in names
unless the abbreviation is really unambiguous and obvious to everyone (e.g.,
use ``size`` over ``sz``). Do not use short inappropriate names such as foo,
bar, or baz. The name of an item should always match its purpose. As such,
names such as ``tmp`` to identify a temporary variable, or such as ``i`` to
identify a loop index are OK.

If you use predicates (that is, functions, variables or methods which return
a single boolean value), prefix the name with "is" or "has".

File layout and code organization
=================================

A class named ``MyClass`` should be declared in a header named ``my-class.h``
and implemented in a source file named ``my-class.cc``. The goal of this
naming pattern is to allow a reader to quickly navigate through the |ns3|
codebase to locate the source file relevant to a specific type.

Each ``my-class.h`` header should start with the following comment to ensure
that your code is licensed under the GPL, that the copyright holders are properly
identified (typically, you or your employer), and that the actual author
of the code is identified. The latter is purely informational and we use it
to try to track the most appropriate person to review a patch or fix a bug.
Please do not add the "All Rights Reserved" phrase after the copyright
statement.

.. sourcecode:: cpp

  /*
   * Copyright (c) YEAR COPYRIGHTHOLDER
   *
   * This program is free software; you can redistribute it and/or modify
   * it under the terms of the GNU General Public License version 2 as
   * published by the Free Software Foundation;
   *
   * This program is distributed in the hope that it will be useful,
   * but WITHOUT ANY WARRANTY; without even the implied warranty of
   * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   * GNU General Public License for more details.
   *
   * You should have received a copy of the GNU General Public License
   * along with this program; if not, write to the Free Software
   * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
   *
   * Author: MyName <myemail@example.com>
   */

Below these C-style comments, always include the following which defines a
set of header guards (``MY_CLASS_H``) used to avoid multiple header includes,
which ensures that your code is included in the |ns3| namespace and which
provides a set of Doxygen comments for the public part of your class API.
Detailed information on the set of tags available for doxygen documentation
is described in the `Doxygen website <https://www.doxygen.nl/index.html>`_.

.. sourcecode:: cpp

  #ifndef MY_CLASS_H
  #define MY_CLASS_H

  namespace ns3
  {

  /**
   * \brief short one-line description of the purpose of your class
   *
   * A longer description of the purpose of your class after a blank
   * empty line.
   */
  class MyClass
  {
  public:
      MyClass();

      /**
       * A detailed description of the purpose of the method.
       *
       * \param firstParam a short description of the purpose of this parameter
       * \returns a short description of what is returned from this function.
       */
      int DoSomething(int firstParam);

  private:
      /**
       * Private method doxygen is also recommended
       */
      void MyPrivateMethod();

      int m_myPrivateMemberVariable; ///< Brief description of member variable
  };

  } // namespace ns3

  #endif // MY_CLASS_H

The ``my-class.cc`` file is structured similarly:

.. sourcecode:: cpp

  /*
   * Copyright (c) YEAR COPYRIGHTHOLDER
   *
   * 3-paragraph GPL blurb
   *
   * Author: MyName <myemail@foo.com>
   */

  #include "my-class.h"

  namespace ns3
  {

  MyClass::MyClass()
  {
  }

  ...

  } // namespace ns3

Header file includes
====================

Included header files should be organized by source location. The sorting order is
as follows:

.. sourcecode:: cpp

  // Header class (applicable for *.cc files)
  #include "my-class.h"

  // Includes from the same module
  #include "header-from-same-module.h"

  // Includes from other modules
  #include "ns3/header-from-different-module.h"

  // External headers (e.g., STL libraries)
  #include <iostream>

Groups should be separated by a new line. Within each group, headers should be
sorted alphabetically.

For standard headers, use the C++ style of inclusion:

.. sourcecode:: cpp

  #include <cstring>  // OK
  #include <string.h> // Avoid

- inside .h files, always use

  .. sourcecode:: cpp

    #include <ns3/header.h>

- inside .cc files, use

  .. sourcecode:: cpp

    #include "header.h"

  if file is in same directory, otherwise use

  .. sourcecode:: cpp

    #include <ns3/header.h>

Variables
=========

Each variable declaration is on a separate line.
Variables should be declared at the point in the code where they are needed,
and should be assigned an initial value at the time of declaration.

.. sourcecode:: cpp

  // Do not declare multiple variables per line
  int x, y;

  // Declare one variable per line and assign an initial value
  int x = 0;
  int y = 0;

Comments
========

The project uses `Doxygen <https://www.doxygen.nl/index.html>`_ to document
the interfaces, and uses comments for improving the clarity of the code
internally. All classes, methods, and members should have Doxygen comments.
Doxygen comments should use the C-style comment (also known as Javadoc) style.
For comments that are intended to not be exposed publicly in the Doxygen output,
use the ``@internal`` and ``@endinternal`` tags.
Please use the ``@see`` tag for cross-referencing.
All parameters and return values should be documented. The |ns3| codebase uses
both the ``@`` or ``\`` characters for tag identification; please make sure
that usage is consistent within a file.

.. sourcecode:: cpp

  /**
   * MyClass description.
   */
  class MyClass
  {
      /**
       * Constructor.
       *
       * \param n Number of elements.
       */
      MyClass(int n);
  };


As for comments within the code, comments should be used to describe intention
or algorithmic overview where is it not immediately obvious from reading the
code alone. There are no minimum comment requirements and small routines
probably need no commenting at all, but it is hoped that many larger
routines will have commenting to aid future maintainers. Please write
complete English sentences and capitalize the first word unless a lower-case
identifier begins the sentence. Two spaces after each sentence helps to make
emacs sentence commands work. Sometimes ``NS_LOG_DEBUG`` statements can
be also used in place of comments.

Short one-line comments and long comments can use the C++ comment style;
that is, ``//``, but longer comments may use C-style comments.
Use one space after ``//`` or ``/*``.

.. sourcecode:: cpp

  /*
   * A longer comment,
   * with multiple lines.
   */

Variable declaration should have a short, one or two line comment describing
the purpose of the variable, unless it is a local variable whose use is
obvious from the context. The short comment should be on the same line as the
variable declaration, unless it is too long, in which case it should be on
the preceding lines.

.. sourcecode:: cpp

  int nNodes = 3; // Number of nodes

  /// Node container with the Wi-Fi stations
  NodeContainer wifiStations(3);

Casts
=====

Where casts are necessary, use the Google C++ guidance:  "Use C++-style casts
like ``static_cast<float> (double_value)``, or brace initialization for
conversion of arithmetic types like ``int64 y = int64{1} << 42``."
Do not use C-style casts, since they can be unsafe.

Try to avoid (and remove current instances of) casting of ``uint8_t``
type to larger integers in our logging output by overriding these types
within the logging system itself. Also, the unary ``+`` operator can be used
to print the numeric value of any variable, such as:

.. sourcecode:: cpp

  uint8_t flags = 5;
  std::cout << "Flags numeric value: " << +flags << std::endl;

Avoid unnecessary casts if minor changes to variable declarations can solve
the issue. In the following example, ``x`` can be declared as ``float`` instead of
``int`` to avoid the cast:

.. sourcecode:: cpp

  // Do not declare x as int, to avoid casting it to float
  int x = 3;
  return 1 / static_cast<float>(x);

  // Prefer to declare x as float
  float x = 3.0;
  return 1 / x;

Namespaces
==========

Code should always be included in a given namespace, namely ``ns3``.
In order to avoid exposing internal symbols, consider placing the code in an
anonymous namespace, which can only be accessed by functions in the same file.

Code within namespaces should not be indented. To more easily identify the end
of a namespace, add a trailing comment to its closing brace.

.. sourcecode:: cpp

  namespace ns3
  {

  // (...)

  } // namespace ns3

Namespace names should follow the snake_case convention.

Unused variables
================

Compilers will typically issue warnings on unused entities (e.g., variables,
function parameters). Use the ``[[maybe_unused]]`` attribute to suppress
such warnings when the entity may be unused depending on how the code
is compiled (e.g., if the entity is only used in a logging statement or
an assert statement).

The general guidelines are as follows:

- If a function's or a method's parameter is definitely unused,
  prefer to leave it unnamed. In the following example, the second
  parameter is unnamed.

  .. sourcecode:: cpp

    void
    UanMacAloha::RxPacketGood(Ptr<Packet> pkt, double, UanTxMode txMode)
    {
        UanHeaderCommon header;
        pkt->RemoveHeader(header);
        ...
    }

  In this case, the parameter is also not referenced by Doxygen; e.g.,:

  .. sourcecode:: cpp

    /**
     * Receive packet from lower layer (passed to PHY as callback).
     *
     * \param pkt Packet being received.
     * \param txMode Mode of received packet.
     */
    void RxPacketGood(Ptr<Packet> pkt, double, UanTxMode txMode);

  The omission is preferred to commenting out unused parameters, such as:

  .. sourcecode:: cpp

    void
    UanMacAloha::RxPacketGood(Ptr<Packet> pkt, double /*sinr*/, UanTxMode txMode)
    {
        UanHeaderCommon header;
        pkt->RemoveHeader(header);
        ...
    }

- If a function's parameter is only used in certain cases (e.g., logging),
  or it is part of the function's Doxygen, mark it as ``[[maybe_unused]]``.

  .. sourcecode:: cpp

    void
    TcpSocketBase::CompleteFork(Ptr<Packet> p [[maybe_unused]],
                                const TcpHeader& h,
                                const Address& fromAddress,
                                const Address& toAddress)
    {
        NS_LOG_FUNCTION(this << p << h << fromAddress << toAddress);

        // Remaining code that definitely uses 'h', 'fromAddress' and 'toAddress'
        ...
    }

- If a local variable saves the result of a function that must always run,
  but whose value may not be used, declare it ``[[maybe_unused]]``.

  .. sourcecode:: cpp

    void
    MyFunction()
    {
        int result [[maybe_unused]] = MandatoryFunction();
        NS_LOG_DEBUG("result = " << result);
    }

- If a local variable saves the result of a function that is only run in
  certain cases, prefer to not declare the variable and use the function's
  return value directly where needed. This avoids unnecessarily calling the
  function if its result is not used.

  .. sourcecode:: cpp

    void
    MyFunction()
    {
        // Prefer to call GetDebugInfo() directly on the log statement
        NS_LOG_DEBUG("Debug information: " << GetDebugInfo());

        // Avoid declaring a local variable with the result of GetDebugInfo()
        int debugInfo [[maybe_unused]] = GetDebugInfo();
        NS_LOG_DEBUG("Debug information: " << debugInfo);
    }

  If the calculation of the maybe unused variable is complex, consider wrapping
  the calculation of its value in a conditional block that is only run if the
  variable is used.

  .. sourcecode:: cpp

    if (g_log.IsEnabled(ns3::LOG_DEBUG))
    {
        auto debugInfo = GetDebugInfo();
        auto value = DoComplexCalculation(debugInfo);

        NS_LOG_DEBUG("The value is " << value);
    }

Unnecessary else after return
=============================

In order to increase readability and avoid deep code nests, consider not adding
an ``else`` block if the ``if`` block breaks the control flow (i.e., when using
``return``, ``break``, ``continue``, etc.).

For instance, the following code:

.. sourcecode:: cpp

  if (n < 0)
  {
      return false;
  }
  else
  {
      n += 3;
      return n;
  }

can be rewritten as:

.. sourcecode:: cpp

  if (n < 0)
  {
      return n;
  }

  n += 3;
  return n;

Smart pointer boolean comparisons
=================================

As explained in this `issue <https://gitlab.com/nsnam/ns-3-dev/-/merge_requests/732>`_,
the |ns3| smart pointer class ``Ptr`` should be used in boolean comparisons as follows:

.. sourcecode:: cpp

  for Ptr<> p, do not use:            use instead:
  ========================            =================================
  if (p != nullptr) {...}             if (p)      {...}
  if (p != NULL)    {...}
  if (p != 0)       {...}             if (p)      {...}

  if (p == nullptr) {...}             if (!p)     {...}
  if (p == NULL)    {...}
  if (p == 0)       {...}

  NS_ASSERT... (p != nullptr, ...)    NS_ASSERT... (p, ...)
  NS_ABORT...  (p != nullptr, ...)    NS_ABORT...  (p, ...)

  NS_ASSERT... (p == nullptr, ...)    NS_ASSERT... (!p, ...)
  NS_ABORT...  (p == nullptr, ...)    NS_ABORT...  (!p, ...)

  NS_TEST...   (p, nullptr, ...)      NS_TEST...   (p, nullptr, ...)

C++ standard
============

As of ns-3.36, |ns3| permits the use of C++-17 (or earlier) features
in the implementation files.

If a developer would like to propose to raise this bar to include more
features than this, please email the developers list. We will move this
language support forward as our minimally supported compiler moves forward.

Miscellaneous items
===================

- ``NS_LOG_COMPONENT_DEFINE("log-component-name");`` statements should be
  placed within namespace ns3 (for module code) and after the
  ``using namespace ns3;``.  In examples,
  ``NS_OBJECT_ENSURE_REGISTERED()`` should also be placed within namespace ``ns3``.

- Pointers and references are left-aligned:

  .. sourcecode:: cpp

    int x = 1;
    int* ptr = &x;
    int& ref = x;

- Const reference syntax:

  .. sourcecode:: cpp

    void MySub(const T& t);  // OK
    void MySub(T const& t);  // Not OK

- Do not include inline implementations in header files; put all
  implementation in a ``.cc`` file (unless implementation in the header file
  brings demonstrable and significant performance improvement).

- Do not use ``NULL``, ``nil`` or ``0`` constants; use ``nullptr`` (improves portability)

- Consider whether you want the default constructor, copy constructor, or assignment
  operator in your class. If not, explicitly mark them as deleted and make the
  declaration public:

  .. sourcecode:: cpp

    class MyClass
    {
      public:
        // Allowed constructors
        MyClass(int i);

        // Deleted constructors.
        // Explain why they are not supported.
        MyClass() = delete;
        MyClass(const MyClass&) = delete;
        MyClass& operator=(const MyClass&) = delete;
    };

- Avoid returning a reference to an internal or local member of an object:

  .. sourcecode:: cpp

    MyType& foo();        // Avoid. Prefer to return a pointer or an object.
    const MyType& foo();  // Same as above.

  This guidance does not apply to the use of references to implement operators.

- Expose class members through access functions, rather than direct access
  to a public object. The access functions are typically named ``Get`` and
  ``Set`` followed by the member's name. For example, a member ``m_delayTime``
  might have accessor functions ``GetDelayTime()`` and ``SetDelayTime()``.

- Do not bring the C++ standard library namespace into |ns3| source files by
  using the ``using namespace std;`` directive.

- Do not use the C++ ``goto`` statement.

Clang-tidy rules
================

Please refer to the ``.clang-tidy`` file in the |ns3| root directory for the full list
of rules that should be observed while developing code.

- Explicitly mark inherited functions with the ``override`` specifier.

- Prefer to use ``.emplace_back()`` over ``.push_back()`` to optimize performance.

- When creating STL smart pointers, prefer to use ``std::make_shared`` or
  ``std::make_unique``, instead of creating the smart pointer with ``new``.

  .. sourcecode:: cpp

    auto node = std::make_shared<Node>();           // OK
    auto node = std::shared_ptr<Node>(new Node());  // Avoid

- When looping through containers, prefer to use range-based for loops rather than
  index-based loops.

  .. sourcecode:: cpp

    std::vector<int> myVector{1, 2, 3};

    for (const auto& v : myVector) { ... }             // Prefer
    for (int i = 0; i < myVector.size(); i++) { ... }  // Avoid

- When looping through containers, prefer to use const-ref syntax over copying
  elements.

  .. sourcecode:: cpp

    std::vector<int> myVector{1, 2, 3};

    for (const auto& v : myVector) { ... }  // OK
    for (auto v : myVector) { ... }         // Avoid

- When initializing ``std::vector`` containers with known size, reserve memory to
  store all items, before pushing them in a loop.

  .. sourcecode:: cpp

    constexpr int N_ITEMS = 5;

    std::vector<int> myVector;
    myVector.reserve(N_ITEMS); // Reserve memory to store all items

    for (int i = 0; i < N_ITEMS; i++)
    {
        myVector.emplace_back(i);
    }

- Prefer to initialize STL containers (e.g., ``std::vector``, ``std::map``, etc.)
  directly with a braced-init-list, instead of pushing elements one-by-one.

  .. sourcecode:: cpp

    // OK
    std::vector<int> myVector{1, 2, 3};

    // Avoid
    std::vector<int> myVector;
    myVector.reserve(3);
    myVector.emplace_back(1);
    myVector.emplace_back(2);
    myVector.emplace_back(3);

- Avoid unnecessary calls to the functions ``.c_str()`` and ``.data()`` of
  ``std::string``.

- Avoid unnecessarily dereferencing std smart pointers (``std::shared_ptr``,
  ``std::unique_ptr``) with calls to their member function ``.get()``.
  Prefer to use the std smart pointer directly where needed.

  .. sourcecode:: cpp

    auto ptr = std::make_shared<Node>();

    // OK
    if (ptr) { ... }

    // Avoid
    if (ptr.get()) { ... }

- Avoid declaring trivial destructors, to optimize performance.

- Prefer to use ``static_assert()`` over ``NS_ASSERT()`` when conditions can be
  evaluated at compile-time.
