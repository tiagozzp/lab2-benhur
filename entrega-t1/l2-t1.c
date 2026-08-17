#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define POS_DIA 10
#define POS_NOITE 5
#define NUM_ESCUDOS 3
#define TIROS_INICIAL 30
#define ATAQUES_DIA 20
#define ATAQUES_NOITE 15
#define VAZIO (-1)
#define TIPO_N 10
#define TIPO_n 11
#define ARQ_PONTOS "pontuacoes.txt"
#define NUM_MELHORES 3

typedef struct timespec crono;

typedef struct
{
	bool joga;
	bool onda;
	bool fim;
	bool diurno;

	int pontos;
	int municao;
	int escudos;
	int arma;

	int num_pos;
	int inimigos[POS_DIA];
	int inativos;
	int numero_onda;

	crono relogio;
	double intervalo_mov;
	double proximo_mov;
} estado_t;

void configura_terminal();
void normaliza_terminal();
char lechar();
void crono_inicia(crono *c);
double crono_parcial(crono *c);
void inicializa_estado(estado_t *est);
bool sorteia_diurno(int numero_onda);
void inicializa_onda(estado_t *est);
int sorteia_tipo(estado_t *est);
const char *sequencia_armas(estado_t *est);
void processa_teclado(estado_t *est);
void troca_arma(estado_t *est);
void atira(estado_t *est);
int busca_alvo(estado_t *est, char arma_atual);
void destroi_inimigo(estado_t *est, int i);
void toca_sons(const char *nomes[], int n, bool bloqueante);
void toca_som(const char *nome);
const char *nome_som_tipo(int tipo);
const char *nome_som_arma(char arma_c);
void toca_fim_onda();
void toca_fim_partida();
void sonar(estado_t *est);
void processa_tempo(estado_t *est);
void move_inimigos(estado_t *est);
void desloca_inimigos(estado_t *est);
void trata_saida(estado_t *est, int tipo);
void coloca_novo_inimigo(estado_t *est);
void verifica_fim_onda(estado_t *est);
void desenha(estado_t *est);
void desenha_diurno(estado_t *est);
void desenha_noturno(estado_t *est);
void desenha_arma(estado_t *est);
void desenha_escudos(estado_t *est);
void desenha_inimigos(estado_t *est);
void desenha_inimigo(int inimigo);
void joga_onda(estado_t *est);
void finaliza_onda(estado_t *est);
void joga_partida(estado_t *est);
void finaliza_partida(estado_t *est);
void le_pontuacoes(int pontos[NUM_MELHORES]);
void salva_pontuacoes(int pontos[NUM_MELHORES]);
void atualiza_pontuacoes(int pontos_partida);
bool pergunta(const char *texto);
void loop_partidas(estado_t *est);

void configura_terminal()
{
	if (system("stty raw opost -echo min 0 time 1") != 0) {
		perror("erro na execução de system(\"stty\")");
		exit(1);
	}

	if (setvbuf(stdin, NULL, _IONBF, 0) != 0) {
		perror("erro na execução de setvbuf()");
		exit(1);
	}
}

void normaliza_terminal()
{
	system("stty sane");
}

char lechar()
{
	char c;

	fflush(stdout);

	if (fread(&c, 1, 1, stdin) == 1) {
		return c;
	}

	return 0;
}

void crono_inicia(crono *c)
{
	clock_gettime(CLOCK_MONOTONIC, c);
}

double crono_parcial(crono *c)
{
	crono agora;
	double segundos;
	double nanosegundos;

	clock_gettime(CLOCK_MONOTONIC, &agora);

	segundos = agora.tv_sec - c->tv_sec;
	nanosegundos = agora.tv_nsec - c->tv_nsec;

	return segundos + 1e-9 * nanosegundos;
}

void inicializa_estado(estado_t *est)
{
	est->joga = true;
	est->fim = false;
	est->pontos = 0;
	est->numero_onda = 0;
}

bool sorteia_diurno(int numero_onda)
{
	int chances[4] = { 100, 80, 60, 40 };
	int chance = (numero_onda < 4) ? chances[numero_onda] : 20;

	return (rand() % 100) < chance;
}

void inicializa_onda(estado_t *est)
{
	est->numero_onda++;
	est->diurno = sorteia_diurno(est->numero_onda - 1);
	est->onda = true;
	est->municao = TIROS_INICIAL;
	est->escudos = NUM_ESCUDOS;
	est->arma = 0;

	est->num_pos = est->diurno ? POS_DIA : POS_NOITE;
	est->inativos = est->diurno ? ATAQUES_DIA : ATAQUES_NOITE;

	for (int i = 0; i < POS_DIA; i++) {
		est->inimigos[i] = VAZIO;
	}

	double base = est->diurno ? 2.0 : 6.0;
	for (int i = 0; i < est->numero_onda - 1; i++) {
		base *= 0.9;
	}
	est->intervalo_mov = base;
	est->proximo_mov = base;

	coloca_novo_inimigo(est);
	crono_inicia(&est->relogio);
}

int sorteia_tipo(estado_t *est)
{
	int dia[11] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, TIPO_N };
	int noite[6] = { 0, 2, 4, 6, 8, TIPO_N };

	if (est->diurno) {
		return dia[rand() % 11];
	}
	return noite[rand() % 6];
}

const char *sequencia_armas(estado_t *est)
{
	return est->diurno ? "0123456789n" : "02468n";
}

void processa_teclado(estado_t *est)
{
	char c = lechar();

	if (c == 27) {
		est->joga = false;
		est->onda = false;
	} else if (c == 9) {
		troca_arma(est);
	} else if (c == 13) {
		atira(est);
	} else if (c == 32) {
		sonar(est);
	}
}

void troca_arma(estado_t *est)
{
	int qtd = strlen(sequencia_armas(est));

	est->arma = (est->arma + 1) % qtd;
	toca_som("arma");
}

int busca_alvo(estado_t *est, char arma_atual)
{
	for (int i = 0; i < est->num_pos; i++) {
		int tipo = est->inimigos[i];

		if (tipo == VAZIO) {
			continue;
		}
		if (arma_atual == 'n' && (tipo == TIPO_N || tipo == TIPO_n)) {
			return i;
		}
		if (arma_atual != 'n' && tipo == arma_atual - '0') {
			return i;
		}
	}

	return VAZIO;
}

void atira(estado_t *est)
{
	char arma_atual;
	int i;

	if (est->municao <= 0) {
		return;
	}
	est->municao--;

	arma_atual = sequencia_armas(est)[est->arma];
	i = busca_alvo(est, arma_atual);

	if (i == VAZIO) {
		toca_som("10");
		return;
	}

	if (arma_atual == 'n' && est->inimigos[i] == TIPO_N) {
		est->inimigos[i] = TIPO_n;
	} else {
		destroi_inimigo(est, i);
	}
	toca_som(nome_som_arma(arma_atual));
}

void destroi_inimigo(estado_t *est, int i)
{
	int valor = est->num_pos - i;

	if (est->inimigos[i] == TIPO_n) {
		valor *= 2;
	}
	if (!est->diurno) {
		valor *= 2;
	}

	est->pontos += valor;
	est->inimigos[i] = VAZIO;
}

void toca_sons(const char *nomes[], int n, bool bloqueante)
{
	char comando[256];
	int pos = sprintf(comando, "aplay -q");

	for (int i = 0; i < n; i++) {
		pos += sprintf(comando + pos, " sons/%s.3.wav", nomes[i]);
	}
	sprintf(comando + pos, " 2>/dev/null%s", bloqueante ? "" : " &");
	system(comando);
}

void toca_som(const char *nome)
{
	const char *nomes[1] = { nome };

	toca_sons(nomes, 1, false);
}

const char *nome_som_tipo(int tipo)
{
	static char buf[4];

	if (tipo == VAZIO) {
		return "x";
	}
	if (tipo == TIPO_N || tipo == TIPO_n) {
		return "11";
	}
	sprintf(buf, "%d", tipo);
	return buf;
}

const char *nome_som_arma(char arma_c)
{
	static char buf[2];

	if (arma_c == 'n') {
		return "11";
	}
	buf[0] = arma_c;
	buf[1] = 0;
	return buf;
}

void toca_fim_onda()
{
	const char *nomes[3] = { "12", "x", "12" };

	toca_sons(nomes, 3, true);
}

void toca_fim_partida()
{
	const char *nomes[3] = { "11", "11", "12" };

	toca_sons(nomes, 3, true);
}

void sonar(estado_t *est)
{
	for (int i = 0; i < est->escudos; i++) {
		toca_som("12");
	}

	for (int i = 0; i < est->num_pos; i++) {
		toca_som(nome_som_tipo(est->inimigos[i]));
	}
}

void processa_tempo(estado_t *est)
{
	double agora = crono_parcial(&est->relogio);

	if (agora >= est->proximo_mov) {
		move_inimigos(est);
		est->proximo_mov += est->intervalo_mov;
	}
}

void move_inimigos(estado_t *est)
{
	int saiu = est->inimigos[0];

	desloca_inimigos(est);

	if (saiu != VAZIO) {
		trata_saida(est, saiu);
	}

	coloca_novo_inimigo(est);
	verifica_fim_onda(est);
}

void desloca_inimigos(estado_t *est)
{
	for (int i = 0; i < est->num_pos - 1; i++) {
		est->inimigos[i] = est->inimigos[i + 1];
	}
	est->inimigos[est->num_pos - 1] = VAZIO;
}

void trata_saida(estado_t *est, int tipo)
{
	(void)tipo;

	if (est->escudos > 0) {
		est->escudos--;
		toca_som("12");
	} else {
		est->onda = false;
		est->fim = true;
	}
}

void coloca_novo_inimigo(estado_t *est)
{
	int tipo;

	if (est->inativos <= 0) {
		return;
	}
	if (est->inimigos[est->num_pos - 1] != VAZIO) {
		return;
	}

	tipo = sorteia_tipo(est);
	est->inimigos[est->num_pos - 1] = tipo;
	est->inativos--;
	toca_som(nome_som_tipo(tipo));
}

void verifica_fim_onda(estado_t *est)
{
	if (est->inativos > 0) {
		return;
	}

	for (int i = 0; i < est->num_pos; i++) {
		if (est->inimigos[i] != VAZIO) {
			return;
		}
	}

	est->onda = false;
}

void desenha(estado_t *est)
{
	if (est->diurno) {
		desenha_diurno(est);
	} else {
		desenha_noturno(est);
	}
}

void desenha_diurno(estado_t *est)
{
	printf("%3d %2d ", est->pontos, est->municao);
	desenha_arma(est);
	printf(" ");
	desenha_escudos(est);
	desenha_inimigos(est);
	printf("\r");
}

void desenha_noturno(estado_t *est)
{
	printf("%d          \r", est->pontos);
}

void desenha_arma(estado_t *est)
{
	printf("%c", sequencia_armas(est)[est->arma]);
}

void desenha_escudos(estado_t *est)
{
	for (int i = 0; i < NUM_ESCUDOS; i++) {
		printf("%c", i < est->escudos ? ')' : ' ');
	}
}

void desenha_inimigos(estado_t *est)
{
	for (int i = 0; i < est->num_pos; i++) {
		desenha_inimigo(est->inimigos[i]);
	}
}

void desenha_inimigo(int inimigo)
{
	if (inimigo == VAZIO) {
		printf(" ");
	} else if (inimigo == TIPO_N) {
		printf("N");
	} else if (inimigo == TIPO_n) {
		printf("n");
	} else {
		printf("%d", inimigo);
	}
}

void joga_onda(estado_t *est)
{
	while (est->onda && est->joga) {
		processa_teclado(est);
		processa_tempo(est);
		desenha(est);
	}

	if (est->joga && !est->fim) {
		finaliza_onda(est);
	}
}

void finaliza_onda(estado_t *est)
{
	int mult = est->diurno ? 1 : 2;
	int bonus = est->municao * 2 + est->escudos * 10;
	char c;

	est->pontos += bonus * mult;
	toca_fim_onda();

	printf("\r\nfim da onda %d - pontos: %d\r\n", est->numero_onda,
	       est->pontos);
	printf("tecle 'r' para continuar...\r\n");

	do {
		c = lechar();
	} while (c != 'r' && est->joga);
}

void joga_partida(estado_t *est)
{
	while (est->joga && !est->fim) {
		inicializa_onda(est);
		joga_onda(est);
	}

	if (est->joga) {
		finaliza_partida(est);
	}
}

void finaliza_partida(estado_t *est)
{
	toca_fim_partida();
	printf("\r\nfim de jogo! pontuação final: %d\r\n", est->pontos);
	atualiza_pontuacoes(est->pontos);
	est->joga = pergunta("jogar novamente? (s/n) ");
}

void le_pontuacoes(int pontos[NUM_MELHORES])
{
	FILE *f;

	for (int i = 0; i < NUM_MELHORES; i++) {
		pontos[i] = 0;
	}

	f = fopen(ARQ_PONTOS, "r");
	if (f == NULL) {
		return;
	}

	for (int i = 0; i < NUM_MELHORES; i++) {
		if (fscanf(f, "%d", &pontos[i]) != 1) {
			break;
		}
	}
	fclose(f);
}

void salva_pontuacoes(int pontos[NUM_MELHORES])
{
	FILE *f = fopen(ARQ_PONTOS, "w");

	if (f == NULL) {
		return;
	}

	for (int i = 0; i < NUM_MELHORES; i++) {
		fprintf(f, "%d\n", pontos[i]);
	}
	fclose(f);
}

void atualiza_pontuacoes(int pontos_partida)
{
	int pontos[NUM_MELHORES];

	le_pontuacoes(pontos);

	for (int i = 0; i < NUM_MELHORES; i++) {
		if (pontos_partida > pontos[i]) {
			for (int j = NUM_MELHORES - 1; j > i; j--) {
				pontos[j] = pontos[j - 1];
			}
			pontos[i] = pontos_partida;
			break;
		}
	}

	salva_pontuacoes(pontos);

	printf("melhores pontuações:\r\n");
	for (int i = 0; i < NUM_MELHORES; i++) {
		printf("  %d\r\n", pontos[i]);
	}
}

bool pergunta(const char *texto)
{
	char c;

	printf("%s", texto);

	do {
		c = lechar();
	} while (c != 's' && c != 'n');

	printf("\r\n");
	return c == 's';
}

void loop_partidas(estado_t *est)
{
	while (est->joga) {
		inicializa_estado(est);
		joga_partida(est);
	}
}

int main()
{
	estado_t estado;

	srand(time(NULL));
	estado.joga = true;

	configura_terminal();
	loop_partidas(&estado);
	normaliza_terminal();

	return 0;
}