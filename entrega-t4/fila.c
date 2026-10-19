#include "fila.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct no {
    void *dado;
    struct no *ant;
    struct no *prox;
} No;

struct fila {
    int tam_do_dado;
    No *inicio;
    No *fim;
    No *percurso_atual;
    int percurso_sentido; 
};

Fila f_cria(int tam_do_dado)
{
    if (tam_do_dado <= 0) {
        return NULL;
    }

    Fila f = malloc(sizeof(*f));
    if (f == NULL) {
        return NULL;
    }

    f->tam_do_dado = tam_do_dado;
    f->inicio = NULL;
    f->fim = NULL;
    f->percurso_atual = NULL;
    f->percurso_sentido = 1;
    return f;
}

void f_destrói(Fila self)
{
    if (self == NULL) {
        return;
    }

    No *atual = self->inicio;
    while (atual != NULL) {
        No *prox = atual->prox;
        free(atual->dado);
        free(atual);
        atual = prox;
    }

    free(self);
}

bool f_tá_vazia(Fila self)
{
    return self == NULL || self->inicio == NULL;
}

void f_remove(Fila self, void *pdado)
{
    if (self == NULL || self->inicio == NULL) {
        return;
    }

    No *removido = self->inicio;
    if (pdado != NULL) {
        memcpy(pdado, removido->dado, self->tam_do_dado);
    }

    self->inicio = removido->prox;
    if (self->inicio != NULL) {
        self->inicio->ant = NULL;
    } else {
        self->fim = NULL;
    }

    if (self->percurso_atual == removido) {
        self->percurso_atual = self->percurso_sentido > 0 ? removido->prox : removido->ant;
    }

    free(removido->dado);
    free(removido);
}

void f_insere(Fila self, void *pdado)
{
    if (self == NULL || pdado == NULL) {
        return;
    }

    No *novo = malloc(sizeof(*novo));
    if (novo == NULL) {
        return;
    }

    novo->dado = malloc(self->tam_do_dado);
    if (novo->dado == NULL) {
        free(novo);
        return;
    }

    memcpy(novo->dado, pdado, self->tam_do_dado);
    novo->ant = self->fim;
    novo->prox = NULL;

    if (self->fim != NULL) {
        self->fim->prox = novo;
    } else {
        self->inicio = novo;
    }

    self->fim = novo;
}

void f_inicia_percurso(Fila self, int pos_inicial)
{
    if (self == NULL) {
        return;
    }

    self->percurso_atual = NULL;
    self->percurso_sentido = 1;

    if (self->inicio == NULL) {
        return;
    }

    if (pos_inicial >= 0) {
        self->percurso_sentido = 1;
        self->percurso_atual = self->inicio;
        for (int i = 0; i < pos_inicial && self->percurso_atual != NULL; i++) {
            self->percurso_atual = self->percurso_atual->prox;
        }
        return;
    }

    self->percurso_sentido = -1;
    self->percurso_atual = self->fim;
    for (int i = 0; i < -pos_inicial - 1 && self->percurso_atual != NULL; i++) {
        self->percurso_atual = self->percurso_atual->ant;
    }
}

bool f_próximo(Fila self, void *pdado)
{
    if (self == NULL || self->percurso_atual == NULL) {
        return false;
    }

    if (pdado != NULL) {
        memcpy(pdado, self->percurso_atual->dado, self->tam_do_dado);
    }

    if (self->percurso_sentido > 0) {
        self->percurso_atual = self->percurso_atual->prox;
    } else {
        self->percurso_atual = self->percurso_atual->ant;
    }

    return true;
}
