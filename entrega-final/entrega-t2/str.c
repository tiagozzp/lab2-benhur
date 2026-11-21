#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MIN_ALLOC 8

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
    if (tam == 0)
    {
        return s;
    }

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

    s->dados = malloc(s->cap);
    assert(s->dados != NULL);
    memcpy(s->dados, strC, tam);
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

Str s_cria_de_arquivo(char *nome)
{
    Str s = s_cria("");
    FILE *arquivo = fopen(nome, "rb");
    if (arquivo == NULL)
    {
        return s;
    }
    if (fseek(arquivo, 0, SEEK_END) != 0)
    {
        fclose(arquivo);
        return s;
    }

    long tam = ftell(arquivo);
    if (tam < 0)
    {
        fclose(arquivo);
        return s;
    }
    if (fseek(arquivo, 0, SEEK_SET) != 0)
    {
        fclose(arquivo);
        return s;
    }
    if (tam == 0)
    {
        fclose(arquivo);
        return s;
    }

    s->nbytes = tam;
    s->cap = MIN_ALLOC;
    while (s->cap < s->nbytes)
    {
        s->cap *= 2;
    }

    s->dados = malloc(s->cap);
    if (s->dados == NULL)
    {
        fclose(arquivo);
        s->n = 0;
        s->nbytes = 0;
        s->cap = 0;
        return s;
    }

    size_t lidos = fread(s->dados, 1, s->nbytes, arquivo);
    fclose(arquivo);
    if ((long)lidos != tam)
    {
        free(s->dados);
        s->dados = NULL;
        s->n = 0;
        s->nbytes = 0;
        s->cap = 0;
        return s;
    }

    s->n = u8_conta_unichar_nos_bytes(s->nbytes, s->dados);
    if (s->n < 0)
    {
        free(s->dados);
        s->dados = NULL;
        s->n = 0;
        s->nbytes = 0;
        s->cap = 0;
        return s;
    }

    return s;
}

// operações de acesso {{{1

int s_tam(Str_c s)
{
    s_ok(s);
    return s->n;
}

char *s_strc(Str_c s)
{
    s_ok(s);
    char *str = malloc(s->nbytes + 1);
    assert(str != NULL);
    memcpy(str, s->dados, s->nbytes);
    str[s->nbytes] = '\0';
    return str;
}

unichar s_ch(Str_c s, int pos)
{
    s_ok(s);
    if (pos < 0 || pos >= s->n)
    {
        return UNI_INV;
    }

    byte *ptr = u8_avanca_unichar(s->dados, pos);
    unichar c;
    u8_unichar_nos_bytes(4, ptr, &c);
    return c;
}

// operações de busca e comparação {{{1

bool s_igual(Str_c s, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    if (s->n != sb->n) return false;
    if (s->nbytes != sb->nbytes) return false;
    return memcmp(s->dados, sb->dados, s->nbytes) == 0;
}

int s_busca_c(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    if (pos < 0) pos = 0;
    if (pos >= s->n || sb->n == 0) return -1;

    for (int i = pos; i < s->n; i++)
    {
        unichar c = s_ch(s, i);
        for (int j = 0; j < sb->n; j++)
        {
            if (c == s_ch(sb, j)) return i;
        }
    }

    return -1;
}

int s_busca_nc(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    if (pos < 0) pos = 0;
    if (pos >= s->n) return -1;

    for (int i = pos; i < s->n; i++)
    {
        unichar c = s_ch(s, i);
        bool pertence = false;
        for (int j = 0; j < sb->n; j++)
        {
            if (c == s_ch(sb, j))
            {
                pertence = true;
                break;
            }
        }
        if (!pertence) return i;
    }

    return -1;
}

int s_busca_rc(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    if (pos < 0) pos = 0;
    if (pos >= s->n || sb->n == 0) return -1;

    for (int i = pos - 1; i >= 0; i--)
    {
        unichar c = s_ch(s, i);
        for (int j = 0; j < sb->n; j++)
        {
            if (c == s_ch(sb, j)) return i;
        }
    }

    return -1;
}

int s_busca_rnc(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    if (pos > s->n) pos = s->n;
    if (pos <= 0) return -1;

    for (int i = pos - 1; i >= 0; i--)
    {
        unichar c = s_ch(s, i);
        bool pertence = false;
        for (int j = 0; j < sb->n; j++)
        {
            if (c == s_ch(sb, j))
            {
                pertence = true;
                break;
            }
        }
        if (!pertence) return i;
    }

    return -1;
}

int s_busca_s(Str_c s, int pos, Str_c buscada)
{
    s_ok(s);
    s_ok(buscada);
    if (pos < 0) pos = 0;
    if (pos > s->n) pos = s->n;
    if (buscada->n == 0) return pos;

    for (int i = pos; i <= s->n - buscada->n; i++)
    {
        bool igual = true;
        for (int j = 0; j < buscada->n; j++)
        {
            if (s_ch(s, i + j) != s_ch(buscada, j))
            {
                igual = false;
                break;
            }
        }
        if (igual) return i;
    }

    return -1;
}

// operações de alteração {{{1

void s_substitui(Str s, int pos, int tam, Str_c sb)
{
    s_ok(s);

    if (sb != NULL)
    {
        s_ok(sb);
    }

    int inicio;
    if (pos < 0)
    {
        inicio = s->n + pos + 1;
    }
    else
    {
        inicio = pos;
    }

    if (inicio < 0) inicio = 0;
    if (inicio > s->n) inicio = s->n;

    int fim;
    if (tam < 0)
    {
        fim = s->n;
    }
    else
    {
        fim = inicio + tam;
    }

    if (fim < inicio) fim = inicio;
    if (fim > s->n) fim = s->n;

    pos = inicio;
    tam = fim - inicio;

    int byte_ini = 0;
    int byte_fim = 0;

    if (pos > 0)
    {
        byte_ini = (int)(u8_avanca_unichar(s->dados, pos) - s->dados);
    }
    if (pos + tam > 0)
    {
        byte_fim = (int)(u8_avanca_unichar(s->dados, pos + tam) - s->dados);
    }

    int bytes_removidos = byte_fim - byte_ini;
    int bytes_inseridos = 0;
    int chars_inseridos = 0;

    if (sb != NULL)
    {
        bytes_inseridos = sb->nbytes;
        chars_inseridos = sb->n;
    }

    int novo_nbytes = s->nbytes - bytes_removidos + bytes_inseridos;
    int novo_n = s->n - tam + chars_inseridos;

    if (novo_nbytes == 0)
    {
        free(s->dados);
        s->dados = NULL;
        s->n = 0;
        s->nbytes = 0;
        s->cap = 0;
        return;
    }

    int nova_cap = MIN_ALLOC;
    while (nova_cap < novo_nbytes)
    {
        nova_cap *= 2;
    }

    int cauda = s->nbytes - byte_fim;

    if (nova_cap > s->cap)
    {
        byte *p = realloc(s->dados, nova_cap);
        assert(p != NULL);
        s->dados = p;
        s->cap = nova_cap;
    }

    memmove(
        s->dados + byte_ini + bytes_inseridos,
        s->dados + byte_fim,
        cauda
    );

    if (sb != NULL && bytes_inseridos > 0)
    {
        memcpy(s->dados + byte_ini, sb->dados, bytes_inseridos);
    }

    if (nova_cap < s->cap)
    {
        byte *p = realloc(s->dados, nova_cap);
        assert(p != NULL);
        s->dados = p;
        s->cap = nova_cap;
    }

    s->n = novo_n;
    s->nbytes = novo_nbytes;
}

void s_substring(Str s, Str_c sb, int pos, int tam)
{
    s_ok(s);
    s_ok(sb);

    int inicio = (pos < 0) ? (sb->n + pos + 1) : pos;
    int fim = (tam < 0) ? sb->n : (inicio + tam);

    if (inicio < 0) inicio = 0;
    if (inicio > sb->n) inicio = sb->n;
    if (fim < 0) fim = 0;
    if (fim > sb->n) fim = sb->n;
    if (fim < inicio) fim = inicio;

    pos = inicio;
    tam = fim - inicio;

    if (tam == 0)
    {
        free(s->dados);
        s->dados = NULL;
        s->n = 0;
        s->nbytes = 0;
        s->cap = 0;
        return;
    }

    int byte_ini = 0;
    int byte_fim = 0;

    if (pos > 0)
    {
        byte *p = u8_avanca_unichar(sb->dados, pos);
        byte_ini = (int)(p - sb->dados);
    }

    byte *pf = u8_avanca_unichar(sb->dados, pos + tam);
    byte_fim = (int)(pf - sb->dados);

    int nbytes = byte_fim - byte_ini;

    int nova_cap = MIN_ALLOC;
    while (nova_cap < nbytes)
    {
        nova_cap *= 2;
    }

    byte *dados = malloc(nova_cap);
    assert(dados != NULL);

    memcpy(dados, sb->dados + byte_ini, nbytes);

    free(s->dados);
    s->dados = dados;
    s->cap = nova_cap;
    s->n = tam;
    s->nbytes = nbytes;
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
    if (pos < 0) pos = s->n + pos + 1;
    if (pos < 0) pos = 0;
    if (pos > s->n) pos = s->n;

    int byte_ini = 0;
    if (s->n == 0 || pos == 0)
    {
        byte_ini = 0;
    }
    else
    {
        byte *p = u8_avanca_unichar(s->dados, pos);
        byte_ini = (int)(p - s->dados);
    }

    byte buf[4];
    int nb = u8_converte_pra_utf8(c, buf);
    if (nb < 0) return;

    int novo_nbytes = s->nbytes + nb;
    int novo_n = s->n + 1;
    int nova_cap = MIN_ALLOC;
    while (nova_cap < novo_nbytes)
    {
        nova_cap *= 2;
    }

    byte *novos = realloc(s->dados, nova_cap);
    assert(novos != NULL);
    s->dados = novos;
    s->cap = nova_cap;

    memmove(s->dados + byte_ini + nb, s->dados + byte_ini, s->nbytes - byte_ini);
    memcpy(s->dados + byte_ini, buf, nb);

    s->nbytes = novo_nbytes;
    s->n = novo_n;
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

    if (sobras->n == 0) return;

    int inicio = s_busca_nc(s, 0, sobras);
    if (inicio == -1)
    {
        free(s->dados);
        s->dados = NULL;
        s->n = 0;
        s->nbytes = 0;
        s->cap = 0;
        return;
    }

    int fim = s_busca_rnc(s, s->n, sobras);
    if (fim == -1) return;

    int tam = fim - inicio + 1;
    s_substring(s, s, inicio, tam);
}

// operações de E/S {{{1

void s_imprime(Str_c s)
{
    s_ok(s);
    if (s->nbytes > 0 && s->dados != NULL)
    {
        fwrite(s->dados, 1, s->nbytes, stdout);
    }
}

void s_grava_arquivo(Str_c s, char *nome)
{
    s_ok(s);
    FILE *f = fopen(nome, "wb");
    if (f == NULL) return;
    if (s->nbytes > 0 && s->dados != NULL)
    {
        fwrite(s->dados, 1, s->nbytes, f);
    }
    fclose(f);
}