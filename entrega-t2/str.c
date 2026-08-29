// includes, constantes e declarações {{{1
#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MIN_ALLOC 8 // alocação mínima

struct str
{
    int n;
    int nbytes;
    byte *dados;
    int cap;
};

static void s_ok(Str_c s)
{
    if (s == NULL)
    {
        assert(!"string nula");
    }

    if (s->n < 0 || s->nbytes < 0 || s->cap < 0)
    {
        assert(!"valores negativos");
    }

    if (s->n == 0)
    {
        if (s->dados != NULL || s->nbytes != 0 || s->cap != 0)
        {
            assert(!"string vazia inválida");
        }
        return;
    }

    if (s->dados == NULL)
    {
        assert(!"dados nulos em string não vazia");
    }

    if (s->nbytes <= 0)
    {
        assert(!"nbytes inválido");
    }

    if (s->cap < MIN_ALLOC)
    {
        assert(!"capacidade menor que o mínimo");
    }

    if ((s->cap & (s->cap - 1)) != 0)
    {
        assert(!"capacidade não é potência de 2");
    }

    if (s->cap < s->nbytes)
    {
        assert(!"capacidade menor que nbytes");
    }

    if (s->cap > MIN_ALLOC && s->cap > s->nbytes * 3)
    {
        assert(!"capacidade acima do limite");
    }
}

//...

// operações de criação e destruição {{{1

Str s_cria(char const *strC)
{
    Str s = malloc(sizeof(*s));
    assert(s != NULL);
    s->n = 0;
    s->nbytes = 0;
    s->dados = NULL;
    s->cap = 0;
    if (strC == NULL)
    {
        return s;
    }
    int tam = strlen(strC);
    int n = u8_conta_unichar_nos_bytes(tam, (byte *)strC);
    if (n < 0) 
    {
        return s;
    }
    s->n = n;
    s->nbytes = tam;
    s->cap = MIN_ALLOC;
    while (s->cap < s->nbytes) 
    {
        s->cap *= 2;
    }
    if (tam == 0) 
    {
        return s;
    }
    s->dados = malloc(s->cap);
    assert (s->dados != NULL);
    memcpy (s->dados, strC, tam);
    return s;
}

void s_destroi(Str s)
{
    s_ok(s);

    free(s->dados);
    free(s);
}

Str s_cria_substring(Str_c s, int pos, int tam)
{
    Str nova = s_cria("");
    s_substring(nova, s, pos, tam);
    return nova;
}

Str s_cria_copia(Str_c s)
{
    return s_cria_substring(s, 0, -1);
}

// Retorna uma nova string com o conteúdo do arquivo chamado nome.
// Retorna uma string vazia em caso de erro.
Str s_cria_de_arquivo(char *nome)
{
    Str s = s_cria("");
    FILE *arquivo = fopen(nome, "rb");
    if (arquivo == NULL) 
    {
        return s;
    }

    if (fseek ((arquivo, 0, SEEK_END) != 0))
    {
        fclose (arquivo);
        return s;
    }
    

    return s;
}

// operações de acesso {{{1

int s_tam(Str_c s)
{
    s_ok(s);
    //...
    return 0;
}

char *s_strc(Str_c s)
{
    s_ok(s);
    //...
    return NULL;
}

unichar s_ch(Str_c s, int pos)
{
    s_ok(s);
    //...
    return UNI_INV;
}

// operações de busca e comparação {{{1

bool s_igual(Str_c s, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    //...
    return false;
}

int s_busca_c(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    //...
    return -1;
}

int s_busca_nc(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    //...
    return -1;
}

int s_busca_rc(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    //...
    return -1;
}

int s_busca_rnc(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    //...
    return -1;
}

int s_busca_s(Str_c s, int pos, Str_c buscada)
{
    s_ok(s);
    s_ok(buscada);
    //...
    return -1;
}

// operações de alteração {{{1

void s_substitui(Str s, int pos, int tam, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    //...
}

void s_substring(Str s, Str_c sb, int pos, int tam)
{
    s_ok(s);
    s_ok(sb);
    //...
}

void s_copia(Str s, Str_c sb)
{
    s_substring(s, sb, 0, -1);
}

void s_insere(Str s, int pos, Str_c sb)
{
    s_substitui(s, pos, 0, sb);
}

void s_insere_c(Str s, int pos, unichar c)
{
    s_ok(s);
    //...
}

void s_anexa(Str s, Str_c sb)
{
    s_substitui(s, -1, 0, sb);
}

void s_anexa_c(Str s, unichar c)
{
    s_insere_c(s, -1, c);
}

void s_remove(Str s, int pos, int tam)
{
    s_substitui(s, pos, tam, NULL);
}

void s_apara(Str s, Str_c sobras)
{
    s_ok(s);
    s_ok(sobras);
    //...
}

// operações de E/S {{{1

void s_imprime(Str_c s)
{
    s_ok(s);
    //...
}

void s_grava_arquivo(Str_c s, char *nome)
{
    s_ok(s);
    //...
}

// vim: foldmethod=marker shiftwidth=2
