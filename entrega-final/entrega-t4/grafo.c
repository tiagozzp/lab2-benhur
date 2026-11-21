#include "grafo.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct aresta {
    int destino;
    void *valor;
    struct aresta *prox;
};

struct no {
    void *valor;
    struct aresta *arestas;
};

struct grafo {
    struct no *nos;
    int capacidade;
    int n_nos;
    int tam_no;
    int tam_aresta;

    int origem_percurso;
    int destino_percurso;
    struct aresta *proxima_aresta;
};

static bool no_valido(Grafo self, int no)
{
    return self != NULL && no >= 0 && no < self->n_nos;
}

static void libera_arestas(struct aresta *aresta)
{
    while (aresta != NULL) {
        struct aresta *proxima = aresta->prox;
        free(aresta->valor);
        free(aresta);
        aresta = proxima;
    }
}

Grafo g_cria(int tam_nó, int tam_aresta)
{
    if (tam_nó <= 0 || tam_aresta <= 0) {
        return NULL;
    }

    Grafo g = calloc(1, sizeof(*g));
    if (g != NULL) {
        g->tam_no = tam_nó;
        g->tam_aresta = tam_aresta;
        g->destino_percurso = -1;
    }
    return g;
}

void g_destrói(Grafo self)
{
    if (self == NULL) {
        return;
    }

    for (int no = 0; no < self->n_nos; no++) {
        free(self->nos[no].valor);
        libera_arestas(self->nos[no].arestas);
    }
    free(self->nos);
    free(self);
}

int g_insere_nó(Grafo self, void *p_dado)
{
    if (self == NULL || p_dado == NULL || self->n_nos == INT_MAX) {
        return -1;
    }

    if (self->n_nos == self->capacidade) {
        int capacidade = 4;
        if (self->capacidade > 0) {
            capacidade = self->capacidade > INT_MAX / 2
                ? INT_MAX : 2 * self->capacidade;
        }
        if ((size_t)capacidade > SIZE_MAX / sizeof(*self->nos)) {
            return -1;
        }
        struct no *nos = realloc(self->nos, (size_t)capacidade * sizeof(*nos));
        if (nos == NULL) {
            return -1;
        }
        self->nos = nos;
        self->capacidade = capacidade;
    }

    void *valor = malloc(self->tam_no);
    if (valor == NULL) {
        return -1;
    }
    memcpy(valor, p_dado, self->tam_no);
    int indice = self->n_nos++;
    self->nos[indice] = (struct no){ .valor = valor };
    return indice;
}

static struct aresta **localiza_aresta(Grafo self, int origem, int destino)
{
    struct aresta **ligacao = &self->nos[origem].arestas;
    while (*ligacao != NULL && (*ligacao)->destino != destino) {
        ligacao = &(*ligacao)->prox;
    }
    return ligacao;
}

static void remove_aresta(struct aresta **ligacao)
{
    struct aresta *aresta = *ligacao;
    *ligacao = aresta->prox;
    free(aresta->valor);
    free(aresta);
}

static void encerra_percurso(Grafo self)
{
    self->destino_percurso = -1;
    self->proxima_aresta = NULL;
}

void g_remove_nó(Grafo self, int nó)
{
    if (!no_valido(self, nó)) {
        return;
    }

    encerra_percurso(self);
    free(self->nos[nó].valor);
    libera_arestas(self->nos[nó].arestas);
    for (int origem = 0; origem < self->n_nos; origem++) {
        if (origem == nó) {
            continue;
        }
        struct aresta **ligacao = &self->nos[origem].arestas;
        while (*ligacao != NULL) {
            if ((*ligacao)->destino == nó) {
                remove_aresta(ligacao);
                continue;
            }
            if ((*ligacao)->destino > nó) {
                (*ligacao)->destino--;
            }
            ligacao = &(*ligacao)->prox;
        }
    }

    self->n_nos--;
    memmove(&self->nos[nó], &self->nos[nó + 1],
            (size_t)(self->n_nos - nó) * sizeof(*self->nos));
}

void g_altera_valor_do_nó(Grafo self, int nó, void *p_dado)
{
    if (no_valido(self, nó) && p_dado != NULL) {
        memmove(self->nos[nó].valor, p_dado, self->tam_no);
    }
}

void *g_valor_do_nó(Grafo self, int nó)
{
    return no_valido(self, nó) ? self->nos[nó].valor : NULL;
}

int g_num_nós(Grafo self)
{
    return self == NULL ? 0 : self->n_nos;
}

void g_altera_valor_da_aresta(Grafo self, int nó_origem, int nó_destino,
                             void *p_dado)
{
    if (!no_valido(self, nó_origem) || !no_valido(self, nó_destino)) {
        return;
    }

    struct aresta **ligacao = localiza_aresta(self, nó_origem, nó_destino);
    if (p_dado == NULL) {
        if (*ligacao != NULL) {
            if (self->proxima_aresta == *ligacao) {
                self->proxima_aresta = (*ligacao)->prox;
            }
            remove_aresta(ligacao);
        }
        return;
    }

    if (*ligacao == NULL) {
        struct aresta *aresta = malloc(sizeof(*aresta));
        if (aresta == NULL) {
            return;
        }
        aresta->valor = malloc(self->tam_aresta);
        if (aresta->valor == NULL) {
            free(aresta);
            return;
        }
        aresta->destino = nó_destino;
        aresta->prox = NULL;
        *ligacao = aresta;
    }
    memmove((*ligacao)->valor, p_dado, self->tam_aresta);
}

void *g_valor_da_aresta(Grafo self, int nó_origem, int nó_destino)
{
    if (!no_valido(self, nó_origem) || !no_valido(self, nó_destino)) {
        return NULL;
    }
    struct aresta *aresta = *localiza_aresta(self, nó_origem, nó_destino);
    return aresta == NULL ? NULL : aresta->valor;
}

void g_arestas_que_partem(Grafo self, int nó_origem)
{
    if (self == NULL) {
        return;
    }
    encerra_percurso(self);
    self->proxima_aresta = no_valido(self, nó_origem)
        ? self->nos[nó_origem].arestas : NULL;
}

void g_arestas_que_chegam(Grafo self, int nó_destino)
{
    if (self == NULL) {
        return;
    }
    encerra_percurso(self);
    if (no_valido(self, nó_destino)) {
        self->destino_percurso = nó_destino;
        self->origem_percurso = 0;
    }
}

void *g_próxima_aresta(Grafo self, int *p_nó_vizinho)
{
    if (self == NULL) {
        return NULL;
    }

    if (self->destino_percurso >= 0) {
        while (self->origem_percurso < self->n_nos) {
            int origem = self->origem_percurso++;
            struct aresta *aresta =
                *localiza_aresta(self, origem, self->destino_percurso);
            if (aresta != NULL) {
                if (p_nó_vizinho != NULL) {
                    *p_nó_vizinho = origem;
                }
                return aresta->valor;
            }
        }
        encerra_percurso(self);
        return NULL;
    }

    struct aresta *aresta = self->proxima_aresta;
    if (aresta == NULL) {
        return NULL;
    }
    self->proxima_aresta = aresta->prox;
    if (p_nó_vizinho != NULL) {
        *p_nó_vizinho = aresta->destino;
    }
    return aresta->valor;
}

static int percorre_topologicamente(Grafo self, Fila ordem)
{
    int total = g_num_nós(self);
    if (total == 0) {
        return 0;
    }

    int *graus = calloc((size_t)total, sizeof(*graus));
    int *pendentes = calloc((size_t)total, sizeof(*pendentes));
    if (graus == NULL || pendentes == NULL) {
        free(graus);
        free(pendentes);
        return -1;
    }

    for (int no = 0; no < total; no++) {
        for (struct aresta *a = self->nos[no].arestas; a != NULL; a = a->prox) {
            graus[a->destino]++;
        }
    }

    int fim = 0;
    for (int no = 0; no < total; no++) {
        if (graus[no] == 0) {
            pendentes[fim++] = no;
        }
    }
    for (int inicio = 0; inicio < fim; inicio++) {
        int no = pendentes[inicio];
        for (struct aresta *a = self->nos[no].arestas; a != NULL; a = a->prox) {
            if (--graus[a->destino] == 0) {
                pendentes[fim++] = a->destino;
            }
        }
    }

    if (ordem != NULL && fim == total) {
        for (int i = 0; i < fim; i++) {
            f_insere(ordem, &pendentes[i]);
        }
    }
    free(graus);
    free(pendentes);
    return fim;
}

bool g_tem_ciclo(Grafo self)
{
    return percorre_topologicamente(self, NULL) != g_num_nós(self);
}

Fila g_ordem_topológica(Grafo self)
{
    Fila ordem = f_cria(sizeof(int));
    if (ordem != NULL && percorre_topologicamente(self, ordem) < 0) {
        f_destrói(ordem);
        return NULL;
    }
    return ordem;
}
