#include "fila.h"

#include <stdbool.h>
#include <stdlib.h>

#define CAPACIDADE_INICIAL 10
#define FATOR_CRESCIMENTO 2

struct fila {
    void *dados;
    int tam_do_dado;
    int capacidade;
    char *inicio;
    char *fim;
    int quantidade;
    char *pos_percurso;
};

Fila f_cria (int tam_do_dado)
{
    Fila nova = (Fila) malloc (sizeof(struct fila));
    nova->dados = NULL;
    nova->tam_do_dado = tam_do_dado;
    nova->capacidade = 0;
    nova->inicio = NULL;
    nova->fim = NULL;
    nova->quantidade = 0;
    nova->pos_percurso = NULL;
    return nova; 
}

void f_destrói (Fila self) 
{
    free (self->dados); 
    free (self);
}

bool f_tá_vazia (Fila self)
{
    return self->quantidade == 0;
}

void f_remove (Fila self, void *pdado)
{
    if (pdado != NULL) 
    {
        memcpy (pdado, self->inicio, self->tam_do_dado);
    }
    self->inicio += self->tam_do_dado;

    self->quantidade--;

    if (self->quantidade == 0) 
    {
        self->inicio = NULL;
        self->fim = NULL;
    }
}

void f_insere (Fila self, void *pdado)
{
    if (self->quantidade == 0) 
    {
        self->capacidade = CAPACIDADE_INICIAL;
    }
    
    if (self->quantidade == self->capacidade) 
    {
        self->capacidade *= FATOR_CRESCIMENTO;
    }
    
    self->dados = realloc(self->dados, self->capacidade * self->tam_do_dado);

    self->inicio = (char *) self->dados;
    self->fim = (char *) self->dados + (self->quantidade * self->tam_do_dado);

    memcpy (self->fim, pdado, self->tam_do_dado);
    self->fim += self->tam_do_dado;
    self->quantidade++;
}

void f_inicia_percurso (Fila self, int pos_inicial)
{
    
}