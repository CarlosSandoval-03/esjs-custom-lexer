/**
 * @file keywords.h
 * @brief EsJS reserved-word inventory and keyword lookup interface.
 *
 * EsJS is a Spanish-syntax dialect of JavaScript. This header centralizes
 * the complete list of reserved words using an X-macro pattern so that the
 * same list can be reused in multiple contexts (string table generation,
 * lookup tables, documentation) without duplication.
 *
 * Usage of the X-macro:
 * @code
 *   // Expand to an array of string literals:
 *   #define MY_MACRO(word) word,
 *   static const char *words[] = { ESJS_KEYWORDS(MY_MACRO) };
 *   #undef MY_MACRO
 * @endcode
 *
 * Source of the reserved word list:
 *   https://es.js.org/sintaxis/palabras-reservadas
 */
#ifndef ESJS_CUSTOM_LEXER_KEYWORDS_H
#define ESJS_CUSTOM_LEXER_KEYWORDS_H

#include "token.h"

/**
 * @brief X-macro that enumerates every EsJS reserved word.
 *
 * Each invocation of the macro @p X receives one string literal argument
 * containing a reserved word. Callers define @p X to perform their desired
 * operation (e.g. emit an array element, a switch case, or a count).
 *
 * The list covers:
 *  - Control-flow keywords: `si`, `sino`, `para`, `mientras`, `hacer`,
 *    `romper`, `continuar`, `elegir`, `caso`, `porDefecto`.
 *  - Exception handling: `intentar`, `capturar`, `finalmente`, `lanzar`.
 *  - Function/generator: `funcion`, `retornar`, `asincrono`, `esperar`,
 *    `producir`.
 *  - Class and OOP: `clase`, `extiende`, `constructor`, `super`, `crear`.
 *  - Module system: `importar`, `exportar`, `desde`, `de`, `en`.
 *  - Variable declarations: `const`, `var`, `mut`.
 *  - Type and introspection operators: `tipoDe`, `instanciaDe`, `vacio`,
 *    `eliminar`.
 *  - Literal values: `verdadero`, `falso`, `nulo`, `indefinido`, `Infinito`,
 *    `NuN`.
 *  - Built-in objects and namespaces: `consola`, `Matriz`, `Cadena`,
 *    `Numero`, `Fecha`, `Promesa`, `Mate`, `Booleano`, `Funcion`.
 *  - Console methods: `escribir`, `advertencia`, `error`, `tabla`, etc.
 *  - String methods: `concatenar`, `incluye`, `reemplazar`, `dividir`, etc.
 *  - Array methods: `mapear`, `filtrar`, `reducir`, `ordenar`, `rodaja`, etc.
 *  - Math functions: `raizCuadrada`, `potencia`, `redondear`, `aleatorio`,
 *    `absoluto`, trigonometric functions, logarithms, etc.
 *  - Date methods: `obtenerDia`, `obtenerMes`, `establecerFecha`, etc.
 *  - Promise methods: `luego`, `rechaza`, `resuelve`, `carrera`, etc.
 *
 * @param X  Macro to apply to each keyword string literal.
 */
#define ESJS_KEYWORDS(X)           \
  X("capturar")                    \
  X("caso")                        \
  X("con")                         \
  X("continuar")                   \
  X("crear")                       \
  X("desde")                       \
  X("elegir")                      \
  X("esperar")                     \
  X("exportar")                    \
  X("hacer")                       \
  X("importar")                    \
  X("mientras")                    \
  X("para")                        \
  X("retornar")                    \
  X("sino")                        \
  X("si")                          \
  X("constructor")                 \
  X("eliminar")                    \
  X("extiende")                    \
  X("finalmente")                  \
  X("instanciaDe")                 \
  X("intentar")                    \
  X("lanzar")                      \
  X("longitud")                    \
  X("romper")                      \
  X("simbolo")                     \
  X("subcad")                      \
  X("tipoDe")                      \
  X("vacio")                       \
  X("producir")                    \
  X("ambiente")                    \
  X("super")                       \
  X("de")                          \
  X("en")                          \
  X("asincrono")                   \
  X("clase")                       \
  X("const")                       \
  X("var")                         \
  X("mut")                         \
  X("porDefecto")                  \
  X("funcion")                     \
  X("falso")                       \
  X("nulo")                        \
  X("verdadero")                   \
  X("indefinido")                  \
  X("Infinito")                    \
  X("NuN")                         \
  X("ambienteGlobal")              \
  X("consola")                     \
  X("depurador")                   \
  X("establecerTemporizador")      \
  X("establecerIntervalo")         \
  X("Fecha")                       \
  X("Numero")                      \
  X("Mate")                        \
  X("Matriz")                      \
  X("Arreglo")                     \
  X("Booleano")                    \
  X("Cadena")                      \
  X("Funcion")                     \
  X("Promesa")                     \
  X("afirmar")                     \
  X("limpiar")                     \
  X("contar")                      \
  X("reiniciarContador")           \
  X("depurar")                     \
  X("listar")                      \
  X("listarXml")                   \
  X("error")                       \
  X("agrupar")                     \
  X("agruparColapsado")            \
  X("finalizarAgrupacion")         \
  X("info")                        \
  X("escribir")                    \
  X("perfil")                      \
  X("finalizarPerfil")             \
  X("tabla")                       \
  X("tiempo")                      \
  X("finalizarTiempo")             \
  X("registrarTiempo")             \
  X("marcaDeTiempo")               \
  X("rastrear")                    \
  X("advertencia")                 \
  X("enPosicion")                  \
  X("caracterEn")                  \
  X("codigoDeCaracterEn")          \
  X("concatenar")                  \
  X("terminaCon")                  \
  X("desdeCodigoDeCaracter")       \
  X("desdePuntoDeCodigo")          \
  X("incluye")                     \
  X("indiceDe")                    \
  X("ultimoIndiceDe")              \
  X("compararLocalizada")          \
  X("coincidir")                   \
  X("coincidirTodo")               \
  X("normalizar")                  \
  X("rellenarAlFinal")             \
  X("rellenarAlComienzo")          \
  X("crudo")                       \
  X("repetir")                     \
  X("reemplazar")                  \
  X("reemplazarTodo")              \
  X("buscarRegex")                 \
  X("recortar")                    \
  X("dividir")                     \
  X("comienzaCon")                 \
  X("subcadena")                   \
  X("aMinusculasLocalizada")       \
  X("aMayusculasLocalizada")       \
  X("aMinusculas")                 \
  X("aMayusculas")                 \
  X("aCadena")                     \
  X("recortarEspacios")            \
  X("recortarEspaciosAlFinal")     \
  X("recortarEspaciosAlComienzo")  \
  X("valorDe")                     \
  X("esNuN")                       \
  X("esFinito")                    \
  X("esEntero")                    \
  X("esEnteroSeguro")              \
  X("interpretarDecimal")          \
  X("interpretarEntero")           \
  X("aExponencial")                \
  X("fijarDecimales")              \
  X("aCadenaLocalizada")           \
  X("aPrecision")                  \
  X("absoluto")                    \
  X("arcocoseno")                  \
  X("arcocosenoHiperbolico")       \
  X("arcoseno")                    \
  X("arcosenoHiperbolico")         \
  X("arcotangente")                \
  X("arcotangente2")               \
  X("arcotangenteHiperbolica")     \
  X("raizCubica")                  \
  X("redondearHaciaArriba")        \
  X("cerosALaIzquierdaEn32Bits")   \
  X("coseno")                      \
  X("cosenoHiperbolico")           \
  X("exponencial")                 \
  X("exponencialMenos1")           \
  X("redondearHaciaAbajo")         \
  X("redondearAComaFlotante")      \
  X("hipotenusa")                  \
  X("multiplicacionEntera")        \
  X("logaritmo")                   \
  X("logaritmoBase10")             \
  X("logaritmoDe1Mas")             \
  X("logaritmoBase2")              \
  X("maximo")                      \
  X("minimo")                      \
  X("potencia")                    \
  X("aleatorio")                   \
  X("redondear")                   \
  X("signo")                       \
  X("seno")                        \
  X("senoHiperbolico")             \
  X("raizCuadrada")                \
  X("tangente")                    \
  X("tangenteHiperbolica")         \
  X("truncar")                     \
  X("obtenerDia")                  \
  X("obtenerDiaSemana")            \
  X("obtenerAnio")                 \
  X("obtenerAño")                  \
  X("obtenerHoras")                \
  X("obtenerMilisegundos")         \
  X("obtenerMinutos")              \
  X("obtenerMes")                  \
  X("obtenerSegundos")             \
  X("obtenerTiempo")               \
  X("obtenerDesfaseDeZonaHoraria") \
  X("obtenerDiaUTC")               \
  X("obtenerDiaSemanaUTC")         \
  X("obtenerAnioUTC")              \
  X("obtenerAñoUTC")               \
  X("obtenerHorasUTC")             \
  X("obtenerMilisegundosUTC")      \
  X("obtenerMinutosUTC")           \
  X("obtenerMesUTC")               \
  X("obtenerSegundosUTC")          \
  X("ahora")                       \
  X("analizar")                    \
  X("establecerFecha")             \
  X("establecerAnio")              \
  X("establecerAño")               \
  X("establecerHoras")             \
  X("establecerMilisegundos")      \
  X("establecerMinutos")           \
  X("establecerMes")               \
  X("establecerSegundos")          \
  X("establecerTiempo")            \
  X("establecerFechaUTC")          \
  X("establecerAnioUTC")           \
  X("establecerAñoUTC")            \
  X("establecerHorasUTC")          \
  X("establecerMilisegundosUTC")   \
  X("establecerMinutosUTC")        \
  X("establecerMesUTC")            \
  X("establecerSegundosUTC")       \
  X("aCadenaFecha")                \
  X("aCadenaISO")                  \
  X("aJSON")                       \
  X("aCadenaFechaLocale")          \
  X("aCadenaLocale")               \
  X("aCadenaTiempoLocale")         \
  X("aCadenaTiempo")               \
  X("aCadenaUTC")                  \
  X("UTC")                         \
  X("posicion")                    \
  X("copiarDentro")                \
  X("entradas")                    \
  X("cada")                        \
  X("llenar")                      \
  X("filtrar")                     \
  X("buscar")                      \
  X("buscarIndice")                \
  X("buscarUltimo")                \
  X("buscarUltimoIndice")          \
  X("plano")                       \
  X("planoMapear")                 \
  X("paraCada")                    \
  X("grupo")                       \
  X("grupoAMapear")                \
  X("mapear")                      \
  X("sacar")                       \
  X("agregar")                     \
  X("reducir")                     \
  X("reducirDerecha")              \
  X("reverso")                     \
  X("sacarPrimero")                \
  X("rodaja")                      \
  X("algun")                       \
  X("ordenar")                     \
  X("empalmar")                    \
  X("agregarInicio")               \
  X("valores")                     \
  X("todos")                       \
  X("todosTerminados")             \
  X("cualquiera")                  \
  X("carrera")                     \
  X("rechaza")                     \
  X("resuelve")                    \
  X("luego")                       \
  X("juntar")                      \
  X("claves")                      \
  X("puntoDeCodigoEn")             \
  X("esNulo")

/**
 * @brief Classifies a lexeme as either a reserved keyword or an identifier.
 *
 * Performs a linear scan of the ESJS_KEYWORDS table. A match requires both
 * equal length and equal byte content (case-sensitive, no null terminator
 * needed in @p lexeme).
 *
 * The function intentionally does not distinguish between specific keyword
 * token types (TOKEN_IF, TOKEN_WHILE, etc.) at this layer; all reserved
 * words return TOKEN_KEYWORD and the caller maps them to finer-grained types
 * if needed.
 *
 * @param lexeme Pointer to the first byte of the candidate lexeme. The
 *               string does not need to be null-terminated.
 * @param length Length of the lexeme in bytes.
 * @return       TOKEN_KEYWORD if @p lexeme matches a reserved word exactly;
 *               TOKEN_IDENTIFIER otherwise.
 */
TokenType keyword_lookup(const char *lexeme, size_t length);

#endif  // ESJS_CUSTOM_LEXER_KEYWORDS_H
