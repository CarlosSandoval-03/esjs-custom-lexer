/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "esjs-custom-lexer", "index.html", [
    [ "esjs-custom-lexer", "d0/d30/md_README.html", [
      [ "Table of Contents", "d0/d30/md_README.html#autotoc_md4", null ],
      [ "Project Overview", "d0/d30/md_README.html#autotoc_md6", null ],
      [ "Architecture", "d0/d30/md_README.html#autotoc_md8", [
        [ "Module Descriptions", "d0/d30/md_README.html#autotoc_md9", null ]
      ] ],
      [ "Processing Flow", "d0/d30/md_README.html#autotoc_md11", null ],
      [ "DFA Design", "d0/d30/md_README.html#autotoc_md13", [
        [ "State Groups", "d0/d30/md_README.html#autotoc_md14", null ],
        [ "Character Classes", "d0/d30/md_README.html#autotoc_md15", null ],
        [ "Accepting States", "d0/d30/md_README.html#autotoc_md16", null ]
      ] ],
      [ "Design Decisions", "d0/d30/md_README.html#autotoc_md18", [
        [ "1. Table-driven DFA over hand-coded switch chains", "d0/d30/md_README.html#autotoc_md19", null ],
        [ "2. Character classes as the DFA alphabet", "d0/d30/md_README.html#autotoc_md20", null ],
        [ "3. Lazy input buffer with full retention", "d0/d30/md_README.html#autotoc_md21", null ],
        [ "4. Maximal munch via last-accepting-state tracking", "d0/d30/md_README.html#autotoc_md22", null ],
        [ "5. Regex / division disambiguation by competitive probing", "d0/d30/md_README.html#autotoc_md23", null ],
        [ "6. Regex entry via a separate state", "d0/d30/md_README.html#autotoc_md24", null ],
        [ "7. Standalone <span class=\"tt\">&amp;</span> and <span class=\"tt\">|</span> are lexical errors", "d0/d30/md_README.html#autotoc_md25", null ],
        [ "8. UTF-8 as a first-class citizen", "d0/d30/md_README.html#autotoc_md26", null ],
        [ "9. X-macro for the keyword list", "d0/d30/md_README.html#autotoc_md27", null ],
        [ "10. Delimiter stripping in the Lexer, not the Scanner", "d0/d30/md_README.html#autotoc_md28", null ],
        [ "11. Unclosed block comment as a lexical error", "d0/d30/md_README.html#autotoc_md29", null ],
        [ "12. Trailing-dot float is not a valid number literal", "d0/d30/md_README.html#autotoc_md30", null ]
      ] ],
      [ "Token Reference", "d0/d30/md_README.html#autotoc_md32", [
        [ "Literal tokens", "d0/d30/md_README.html#autotoc_md33", null ],
        [ "Keyword tokens", "d0/d30/md_README.html#autotoc_md34", null ],
        [ "Operator and punctuation tokens", "d0/d30/md_README.html#autotoc_md35", null ]
      ] ],
      [ "Building and Running", "d0/d30/md_README.html#autotoc_md37", [
        [ "CMake (recommended)", "d0/d30/md_README.html#autotoc_md38", null ],
        [ "GNU Make", "d0/d30/md_README.html#autotoc_md39", null ],
        [ "Dependencies", "d0/d30/md_README.html#autotoc_md40", null ]
      ] ],
      [ "Using as a Library", "d0/d30/md_README.html#autotoc_md42", [
        [ "Quick start", "d0/d30/md_README.html#autotoc_md43", null ],
        [ "Linking with CMake", "d0/d30/md_README.html#autotoc_md44", null ],
        [ "Linking manually (GCC / Make)", "d0/d30/md_README.html#autotoc_md45", null ],
        [ "API summary", "d0/d30/md_README.html#autotoc_md46", null ]
      ] ],
      [ "Output Format", "d0/d30/md_README.html#autotoc_md48", null ],
      [ "License", "d0/d30/md_README.html#autotoc_md50", null ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"db/d62/scanner_8h.html#a85c910a20eb93e6903be833dfb284d6e"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';
var LISTOFALLMEMBERS = 'List of all members';