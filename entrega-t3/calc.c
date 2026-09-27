#include "calc.h"

#include <math.h>
#include <stddef.h>
#include <stdbool.h>

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
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
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

static bool opera(Lista operandos, Str operador)
{
	if (l_tam(operandos) < 2)
		return false;

	Str direita = l_desempilha(operandos);
	Str esquerda = l_desempilha(operandos);
	double a = s_número(esquerda);
	double b = s_número(direita);
	double resultado;
	unichar c = s_ch(operador, 0);

	if (c == '+') resultado = a + b;
	else if (c == '-') resultado = a - b;
	else if (c == '*') resultado = a * b;
	else if (c == '/') resultado = a / b;
	else resultado = pow(a, b);

	s_destroi(esquerda);
	s_destroi(direita);
	l_empilha(operandos, s_cria_número(resultado));
	return true;
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
		else if (e_letra(c) || c == '_' || c == '$')
		{
			while (i < tamanho)
			{
				unichar seguinte = s_ch(txt, i);
				if (!(e_letra(seguinte) || e_digito(seguinte) ||
					  seguinte == '_' || seguinte == '$'))
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

		if (e_numero(token) && espera_operando)
		{
			l_empilha(operandos, s_cria_cópia(token));
			espera_operando = false;
		}
		else if (e_operador(c) && s_tam(token) == 1 && !espera_operando)
		{
			while (!l_vazia(operadores) &&
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
			resultado = l_desempilha(operandos);
	}

	l_destroi(tokens);
	l_destroi(operadores);
	l_destroi(operandos);
	return resultado;
}
