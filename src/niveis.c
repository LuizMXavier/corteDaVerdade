#include "niveis.h"

const int NUM_NIVEIS = 9;

const Nivel NIVEIS[9] = {
    {
        "F1",
        "p v ~p",
        "p: O réu é culpado.",
        "O réu é culpado ou não é culpado.",
        RESULT_TAUTOLOGIA,
        { "p", "v", "~", "p", "p", "v", "~", "p" },
        8
    },
    {
        "F2",
        "~(p ^ ~p)",
        "p: A moção é válida.",
        "Não é verdade que a moção é válida e não é válida.",
        RESULT_TAUTOLOGIA,
        { "p", "^", "~", "p", "~", "p", "^", "p" },
        8
    },
    {
        "F3",
        "p -> p",
        "p: A evidência é admitida.",
        "Se a evidência é admitida, então a evidência é admitida.",
        RESULT_TAUTOLOGIA,
        { "p", "->", "p", "p", "->", "p", "p", "->" },
        8
    },
    {
        "M1",
        "p -> (p v q)",
        "p: A corte está em sessão. q: O caso foi arquivado.",
        "Se a corte está em sessão, então a corte está em sessão ou o caso foi arquivado.",
        RESULT_TAUTOLOGIA,
        { "p", "->", "p", "v", "q", "p", "v", "q" },
        8
    },
    {
        "M2",
        "(p ^ q) -> p",
        "p: O promotor provou o crime. q: O juiz aceitou a prova.",
        "Se o promotor provou o crime e o juiz aceitou a prova,"
        "então o promotor provou o crime.",
        RESULT_TAUTOLOGIA,
        { "p", "^", "q", "->", "p", "p", "^", "q" },
        8
    },
    {
        "M3",
        "[p ^ (p -> q)] -> q",
        "p: A lei é constitucional. q: O réu será condenado.",
        "Se a lei é constitucional e (se a lei é constitucional"
        ",então o réu será condenado),"
        "então o réu será condenado.",
        RESULT_TAUTOLOGIA,
        { "p", "^", "p", "->", "q", "->", "q", "p" },
        8
    },
    {
        "D1",
        "[(p -> q) ^ (q -> r)] -> (p -> r)",
        "p: O depoimento e aceito. q: O juri ouve a prova. r: A decisao e justa.",
        "Se (se o depoimento é aceito, então o juri ouve a prova) "
        "e (se o juri ouve a prova, então a decisão é justa), "
        "então (se o depoimento é aceito, então a decisao é justa). ",
        RESULT_TAUTOLOGIA,
        { "p", "->", "q", "^", "q", "->", "r", "p" },
        8
    },
    {
        "D2",
        "[p -> (q ^ r)] -> (p -> q)",
        "p: O crime foi grave. q: A pena e maxima. r: Nao ha recurso.",
        "Se (se o crime foi grave, então a pena é máxima e não há recurso), "
        "então (se o crime foi grave, então a pena é máxima).",
        RESULT_TAUTOLOGIA,
        { "p", "->", "q", "^", "r", "->", "p", "->" },
        8
    },
    {
        "D3",
        "[p -> (q v r)] <-> [(p ^ ~q) -> r]",
        "p: O juiz decide hoje. q: A defesa apela. r: O reu aguarda em liberdade.",
        "(Se o juiz decide hoje, então a defesa apela ou o réu aguarda em liberdade) "
        "se, e somente se, (se o juiz decide hoje e não a defesa apela, "
        "então o réu aguarda em liberdade).",
        RESULT_TAUTOLOGIA,
        { "p", "->", "q", "v", "r", "<->", "p", "^" },
        8
    }
};
