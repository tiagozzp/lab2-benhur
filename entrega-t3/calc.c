#include "calc.h"
#include "dicionario.h"

#include <math.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static Dicionário variáveis;

static bool chave_menor(chave_t a, chave_t b)
{
	char *texto_a = s_strc(a);
	char *texto_b = s_strc(b);
	bool menor = strcmp(texto_a, texto_b) < 0;
	free(texto_a);
	free(texto_b);
	return menor;
}

static bool chave_igual(chave_t a, chave_t b)
{
	return s_igual(a, b);
}

static Dicionário dicionário_variáveis(void)
{
	if (variáveis == NULL)
		variáveis = dic_cria(chave_menor, chave_igual);
	return variáveis;
}

static bool e_digito(unichar c)
{
	return c >= '0' && c <= '9';
}

static bool e_letra(unichar c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool e_espaco(unichar c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool e_operador(unichar c)
{
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' ||
		   c == '=' || c == '(' || c == ')';
}

static bool e_nome(Str token)
{
	if (s_tam(token) == 0)
		return false;

	unichar inicial = s_ch(token, 0);
	if (!(e_letra(inicial) || inicial == '$'))
		return false;

	for (int i = 1; i < s_tam(token); i++)
	{
		unichar c = s_ch(token, i);
		if (!(e_letra(c) || e_digito(c) || c == '_'))
			return false;
	}
	return true;
}

static bool e_numero(Str token)
{
	bool tem_digito = false;
	int pontos = 0;

	for (int i = 0; i < s_tam(token); i++)
	{
		unichar c = s_ch(token, i);
		if (e_digito(c))
			tem_digito = true;
		else if (c == '.')
			pontos++;
		else
			return false;
	}

	return tem_digito && pontos <= 1;
}

static int precedencia(Str operador)
{
	unichar c = s_ch(operador, 0);
	if (c == '+' || c == '-') return 1;
	if (c == '*' || c == '/') return 2;
	if (c == '^') return 3;
	if (c == '=') return 0;
	return -1;
}

static bool e_abertura(Str operador)
{
	return s_tam(operador) == 1 && s_ch(operador, 0) == '(';
}

static Str erro(const char *mensagem)
{
	Str resultado = s_cria("#ERRO ");
	Str detalhe = s_cria(mensagem);
	s_anexa(resultado, detalhe);
	s_destroi(detalhe);
	return resultado;
}

static bool valor_operando(Str operando, double *valor)
{
	if (e_numero(operando))
	{
		*valor = s_número(operando);
		return true;
	}

	Str armazenado = dic_busca(dicionário_variáveis(), operando);
	if (armazenado == VALOR_NÃO_EXISTE)
		return false;
	*valor = s_número(armazenado);
	return true;
}

static bool atribui(Str nome, double valor)
{
	if (!e_nome(nome))
		return false;

	Dicionário d = dicionário_variáveis();
	Str novo_valor = s_cria_número(valor);
	valor_t anterior = dic_busca(d, nome);
	if (anterior == VALOR_NÃO_EXISTE)
		dic_insere(d, s_cria_cópia(nome), novo_valor);
	else
	{
		dic_insere(d, nome, novo_valor);
		s_destroi(anterior);
	}
	return true;
}

static bool opera(Lista operandos, Str operador)
{
	if (l_tam(operandos) < 2)
		return false;

	Str direita = l_desempilha(operandos);
	Str esquerda = l_desempilha(operandos);
	double a;
	double b;
	unichar c = s_ch(operador, 0);
	bool sucesso;

	if (c == '=')
	{
		sucesso = valor_operando(direita, &b) && atribui(esquerda, b);
		if (sucesso)
		{
			Str valor = s_cria_número(b);
			l_empilha(operandos, valor);
		}
	}
	else
	{
		sucesso = valor_operando(esquerda, &a) && valor_operando(direita, &b);
		if (sucesso)
		{
			double resultado;
			if (c == '+') resultado = a + b;
			else if (c == '-') resultado = a - b;
			else if (c == '*') resultado = a * b;
			else if (c == '/') resultado = a / b;
			else resultado = pow(a, b);
			l_empilha(operandos, s_cria_número(resultado));
		}
	}

	s_destroi(esquerda);
	s_destroi(direita);
	return sucesso;
}

Lista tokeniza(Str txt)
{
	Lista tokens = l_cria();
	int i = 0;
	int tamanho = s_tam(txt);

	while (i < tamanho)
	{
		unichar c = s_ch(txt, i);
		if (e_espaco(c))
		{
			i++;
			continue;
		}

		int inicio = i++;
		if (e_digito(c) || c == '.')
		{
			while (i < tamanho && (e_digito(s_ch(txt, i)) || s_ch(txt, i) == '.'))
				i++;
		}
		else if (e_letra(c) || c == '$')
		{
			while (i < tamanho)
			{
				unichar seguinte = s_ch(txt, i);
				if (!(e_letra(seguinte) || e_digito(seguinte) || seguinte == '_'))
					break;
				i++;
			}
		}

		l_insere_fim(tokens, s_cria_substring(txt, inicio, i - inicio));
	}

	return tokens;
}

Str calculadora(Str expressão)
{
	Lista tokens = tokeniza(expressão);
	Lista operandos = l_cria();
	Lista operadores = l_cria();
	Str resultado = NULL;
	bool espera_operando = true;

	for (int i = 0; i < l_tam(tokens); i++)
	{
		Str token = l_dado_pos(tokens, i);
		unichar c = s_ch(token, 0);

		if (!e_operador(c) && espera_operando)
		{
			l_empilha(operandos, s_cria_cópia(token));
			espera_operando = false;
		}
		else if (e_operador(c) && s_tam(token) == 1 && !espera_operando)
		{
			while (c != '=' && !l_vazia(operadores) &&
				   !e_abertura(l_topo(operadores)) &&
				   precedencia(l_topo(operadores)) >= precedencia(token))
			{
				Str operador = l_desempilha(operadores);
				if (!opera(operandos, operador))
				{
					s_destroi(operador);
					resultado = erro("falta operando");
					break;
				}
				s_destroi(operador);
			}
			if (resultado == NULL)
			{
				l_empilha(operadores, s_cria_cópia(token));
				espera_operando = true;
			}
		}
		else if (c == '(' && s_tam(token) == 1 && espera_operando)
		{
			l_empilha(operadores, s_cria_cópia(token));
		}
		else if (c == ')' && s_tam(token) == 1 && !espera_operando)
		{
			bool encontrou_abertura = false;
			while (!l_vazia(operadores))
			{
				Str operador = l_desempilha(operadores);
				if (s_ch(operador, 0) == '(')
				{
					s_destroi(operador);
					encontrou_abertura = true;
					break;
				}
				if (!opera(operandos, operador))
				{
					s_destroi(operador);
					resultado = erro("falta operando");
					break;
				}
				s_destroi(operador);
			}
			if (!encontrou_abertura)
			{
				resultado = erro("falta (");
			}
			else
				espera_operando = false;
		}
		else
		{
			resultado = erro("token inválido");
		}
		if (resultado != NULL)
			break;
	}
	if (resultado == NULL && espera_operando)
	{
		resultado = erro("expressão inválida");
	}

	while (resultado == NULL && !l_vazia(operadores))
	{
		Str operador = l_desempilha(operadores);
		if (s_ch(operador, 0) == '(')
		{
			s_destroi(operador);
			resultado = erro("falta )");
			break;
		}
		if (!opera(operandos, operador))
		{
			s_destroi(operador);
			resultado = erro("falta operando");
			break;
		}
		s_destroi(operador);
	}

	if (resultado == NULL)
	{
		if (l_tam(operandos) != 1)
			resultado = erro("expressão inválida");
		else
		{
			Str operando = l_desempilha(operandos);
			if (e_numero(operando))
				resultado = operando;
			else
			{
				double valor;
				if (valor_operando(operando, &valor))
					resultado = s_cria_número(valor);
				else
					resultado = erro("operando inválido");
				s_destroi(operando);
			}
		}
	}

	l_destroi(tokens);
	l_destroi(operadores);
	l_destroi(operandos);
	return resultado;
}
