#include "lista.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct no
{
    dado_t dado;
    struct no *anterior;
    struct no *proximo;
};

struct lista
{
    struct no *sentinela;
    int tamanho;
};

static struct no *novo_no(dado_t d)
{
    struct no *no = malloc(sizeof(*no));
    assert(no != NULL);
    no->dado = d;
    return no;
}

static struct no *no_pos(Lista l, int pos)
{
    struct no *no = l->sentinela->proximo;
    for (int i = 0; i < pos; i++)
        no = no->proximo;
    return no;
}

static void insere_antes(struct no *posicao, struct no *novo)
{
    novo->anterior = posicao->anterior;
    novo->proximo = posicao;
    posicao->anterior->proximo = novo;
    posicao->anterior = novo;
}

static dado_t remove_no(struct no *no)
{
    dado_t d = no->dado;
    no->anterior->proximo = no->proximo;
    no->proximo->anterior = no->anterior;
    free(no);
    return d;
}

Lista l_cria() 
{
    Lista l = malloc(sizeof(*l));
    assert(l != NULL);

    l->sentinela = novo_no(NULL);
    l->sentinela->proximo = l->sentinela;
    l->sentinela->anterior = l->sentinela;

    l->tamanho = 0;

    return l;
}

Lista l_cria_separando (Str s, Str sep)
{
    assert(s != NULL);
    assert(sep != NULL);

    Lista l = l_cria();
    int inicio = s_busca_nc(s, 0, sep);

    while (inicio != -1)
    {
        int fim = s_busca_c(s, inicio, sep);
        if (fim == -1) fim = s_tam(s);

        l_insere_fim(l, s_cria_substring(s, inicio, fim - inicio));
        inicio = s_busca_nc(s, fim, sep);
    }

    return l;

}

void l_destroi(Lista l)
{
    assert (l != NULL);

    while (!l_vazia(l))
        s_destroi(l_remove_inicio(l));

    free(l->sentinela);
    free (l);
}

int l_tam (Lista l) 
{
    assert (l != NULL);
    return l-> tamanho;
}

bool l_cheia (Lista l) 
{
    assert (l != NULL);
    return false;
}  

bool l_vazia (Lista l) 
{
    assert (l != NULL);
    return l->tamanho == 0;
}

void l_imprime (Lista l)
{
    assert (l != NULL);
    for (int i = 0; i < l->tamanho; i++) {
        if (i > 0) printf (" ");
        s_imprime (l_dado_pos(l, i));
    }
    printf ("\n");
}

void l_insere_inicio (Lista l, dado_t d) 
{
    assert (l != NULL);
    assert (d != NULL);

    insere_antes(l->sentinela->proximo, novo_no(d));
    l->tamanho++;
}

void l_insere_fim (Lista l, dado_t d)
{
    assert(l != NULL);
    assert(d != NULL);
    insere_antes(l->sentinela, novo_no(d));
    l->tamanho++;
}

void l_insere_pos (Lista l, dado_t d, int p)
{
    assert(l != NULL);
    assert(d != NULL);
    assert(p >= 0 && p <= l->tamanho);
    insere_antes(no_pos(l, p), novo_no(d));
    l->tamanho++;
}

dado_t l_dado_inicio (Lista l)
{
    assert(l != NULL);
    assert(!l_vazia(l));
    return l->sentinela->proximo->dado;
}

dado_t l_dado_fim (Lista l)
{
    assert(l != NULL);
    assert(!l_vazia(l));
    return l->sentinela->anterior->dado;
}

dado_t l_dado_pos (Lista l, int pos)
{
    assert(l != NULL);
    assert(pos >= 0 && pos < l->tamanho);
    return no_pos(l, pos)->dado;
}

dado_t l_remove_inicio (Lista l)
{
    return l_remove_pos(l, 0);
}

dado_t l_remove_fim (Lista l)
{
    assert(l != NULL);
    assert(!l_vazia(l));
    return l_remove_pos(l, l->tamanho - 1);
}

dado_t l_remove_pos (Lista l, int pos)
{
    assert(l != NULL);
    assert(pos >= 0 && pos < l->tamanho);

    dado_t d = remove_no(no_pos(l, pos));
    l->tamanho--;
    return d;
}

dado_t l_primeiro (Lista l)
{
    return l_dado_inicio(l);
}

void l_insere (Lista l, dado_t d)
{
    l_insere_fim(l, d);
}

dado_t l_remove (Lista l)
{
    return l_remove_inicio(l);
}

dado_t l_topo (Lista l)
{
    return l_dado_fim(l);
}

void l_empilha (Lista l, dado_t d)
{
    l_insere_fim(l, d);
}

dado_t l_desempilha (Lista l)
{
    return l_remove_fim(l);
}