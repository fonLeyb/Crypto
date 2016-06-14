#define _CRT_SECURE_NO_WARNINGS

#include "bignum.h"
#include <stdlib.h>
#include <chrono>

#include <iostream>
#include <string>
#include <Windows.h>
#include "change_bit.h"
using namespace std;

// Функция для получения рандома для генерации простых чисел
static int myrand( void *rng_state, unsigned char *output, size_t len )
{
    size_t i;

    if( rng_state != NULL )
        rng_state  = NULL;

    for( i = 0; i < len; ++i )
        output[i] = rand();
    
    return( 0 );
}

// проверка на быстродействие
static void time_check()
{
	mpi GG1, GG2, GG3, GG4; mpi_init(&GG1); mpi_init(&GG2); mpi_init(&GG3); mpi_init(&GG4);
	auto start_time = std::chrono::steady_clock::now();

	mpi_gen_prime(&GG1, 256, 0, myrand, NULL);
	auto end_time1 = std::chrono::steady_clock::now();
	mpi_gen_prime(&GG2, 512, 0, myrand, NULL);
	auto end_time2 = std::chrono::steady_clock::now();
	mpi_gen_prime(&GG3, 1024, 0, myrand, NULL);
	auto end_time3 = std::chrono::steady_clock::now();
	mpi_gen_prime(&GG4, 2048, 0, myrand, NULL);
	auto end_time4 = std::chrono::steady_clock::now();

	auto elapsed_ns1 = std::chrono::duration_cast<std::chrono::milliseconds>(end_time1 - start_time);
	auto elapsed_ns2 = std::chrono::duration_cast<std::chrono::milliseconds>(end_time2 - start_time);
	auto elapsed_ns3 = std::chrono::duration_cast<std::chrono::milliseconds>(end_time3 - start_time);
	auto elapsed_ns4 = std::chrono::duration_cast<std::chrono::milliseconds>(end_time4 - start_time);

	printf(" Время генерации чисел: \n  256 бит: %u \n", elapsed_ns1);
	printf("  512 бит: %u \n", elapsed_ns2);
	printf(" 1024 бит: %u \n", elapsed_ns3);
	printf(" 2048 бит: %u \n", elapsed_ns4);

	mpi_write_file("  (256) GG1 = \n", &GG1, 16, NULL);
	mpi_write_file("  (512) GG2 = \n", &GG2, 16, NULL);
	mpi_write_file(" (1024) GG3 = \n", &GG3, 16, NULL);
	mpi_write_file(" (2048) GG4 = \n", &GG4, 16, NULL);

	mpi_free(&GG1); mpi_free(&GG2); mpi_free(&GG3); mpi_free(&GG4);
}

// генерация параметров
static int generate_parameters(int size, mpi *P, mpi *Q, mpi *N, mpi *H, mpi *G, mpi *E, mpi *D)
{
	printf("\n  Генерация простых чисел P и Q (%i bits): \n", size);
	mpi_gen_prime(P, size, 0, myrand, NULL);
	mpi_gen_prime(Q, size, 0, myrand, NULL);
	mpi_write_file("  P = ", P, 16, NULL);
	mpi_write_file("  Q = ", Q, 16, NULL);

	printf("  Вычисление N = P * Q: \n");
	mpi_mul_mpi(N, P, Q);
	mpi_write_file("  N = ", N, 16, NULL);

	printf("  Вычисление функции Эйлера H = (P - 1) * (Q - 1): \n");
	mpi_sub_int(P, P, 1);
	mpi_sub_int(Q, Q, 1);
	mpi_mul_mpi(H, P, Q);
	mpi_write_file("  H = ", H, 16, NULL);

	printf("  Генерация числа E: \n");
	bool error = true;
	while (error)
	{
		mpi_gen_prime(E, size, 0, myrand, NULL);
		mpi_gcd(G, E, H);
		if (mpi_cmp_abs(G, E) == -1)
			error = false;
	}


	mpi_write_file("  E = ", E, 16, NULL);


	printf("  Вычисление числа D: \n");
	mpi_inv_mod(D, E, H);
	mpi_write_file("  D = ", D, 16, NULL);

	printf("\n  Public key:\n\n");
	mpi_write_file("  E = ", E, 16, NULL);
	mpi_write_file("  N = ", N, 16, NULL);

	printf("\n  Private key:\n\n");
	mpi_write_file("  D = ", D, 16, NULL);
	mpi_write_file("  N = ", N, 16, NULL);
	return 0;
}

/* кодирование
   mpi *Y - исходная строка
   mpi *X - кодированная строка
   mpi *E, mpi *N - пара открытых ключей
*/
static int rsa_enc(mpi *Y, mpi *X, mpi *E, mpi *N)
{
	mpi_exp_mod(Y, X, E, N, NULL);
	return 0;
}
/* декодирование
   mpi *Y - кодированная строка
   mpi *Z - декодированная строка
   mpi *D, mpi *N - пара закрытых ключей
*/
static int rsa_dec(mpi *Z, mpi *Y, mpi *D, mpi *N)
{
	mpi_exp_mod(Z, Y, D, N, NULL);
	return 0;
}
/* создание цифровой подписи
   mpi *S - подпись
   mpi *M - подпись чего создаем
   mpi *D, mpi *N - пара закрытых ключей
*/
static int rsa_crt(mpi *S, mpi *M, mpi *D, mpi *N)
{
	mpi_exp_mod(S, M, D, N, NULL);
	return 0;
}
/* верификация данных
   mpi *MM - прообраз сообщения из подписи
   mpi *S - то, что верифицируем
   mpi *E, mpi *N - пара открытых ключей
*/
static int rsa_ver(mpi *MM, mpi *S, mpi *E, mpi *N)
{
	mpi_exp_mod(MM, S, E, N, NULL);
	return 0;
}



int main(int argc, char* argv[])
{
	mpi E, P, Q, N, H, D, X, Y, Z;


	setlocale(LC_ALL, "Russian");

	//Вызов функции провреки на быстродействие
	//time_check();

	mpi G, M, S, MM;
	
    mpi_init( &E ); mpi_init( &P ); mpi_init( &Q ); mpi_init( &N );
    mpi_init( &H ); mpi_init( &D ); mpi_init( &X ); mpi_init( &Y );
    mpi_init( &Z );


	mpi_init(&G); mpi_init(&M); mpi_init(&S); mpi_init(&MM);

	static int ex = 0;
	static int i = 0;
	static int size_b = 64;
	static int b_size[] = { 8, 16, 32, 64, 128, 256, 512, 1024, 2048 };
	printf("  Введите размер ключа (степень двойки): "); scanf("%i", &size_b);
	for (i = 0; i < 9; i++)
	{
		bool exit = false;
		if (size_b == b_size[i])
		{
			printf("  ~~ размер ключа = %u\n", size_b);
			Sleep(1000);
			exit = true;
		}
		else if (size_b < b_size[i])
		{
			printf("  ~~ размер блока не представлен степенью двойки, исправляем...\n");
			Sleep(1000);
			size_b = b_size[i];
			printf("  ~~ новый размер ключа = %u\n", size_b);
			Sleep(1000);
			exit = true;
		}
		if (exit)
			break;
	}

	// сгенерировать все параметры
	ex = generate_parameters(size_b, &P, &Q, &N, &H, &G, &E, &D);

	printf("  Кодирование: \n\n");

	string msg = "message lolol";
	printf("                  Введите сообщение: ");
	cin.ignore(256, '\n');
	getline(cin, msg);
	const unsigned char* msg_in = (unsigned char*)msg.c_str();
	int msg_len = msg.length();
	mpi_read_binary(&X, msg_in, msg_len);

	printf("\n\n");

	mpi_write_file("             исходный текст: ", &X, 16, NULL);
	ex = rsa_enc(&Y, &X, &E, &N);
	mpi_write_file("         кодированный текст: ", &Y, 16, NULL);
	ex = rsa_dec(&Z, &Y, &D, &N);
	mpi_write_file("      раскодированный текст: ", &Z, 16, NULL);



	printf("\n\n  Цифровая подпись: \n\n");

	mpi_read_binary(&X, msg_in, msg_len);
	mpi_write_file("             исходный текст: ", &X, 16, NULL);
	ex = rsa_crt(&S, &X, &D, &N);
	mpi_write_file("  создание цифровой подписи: ", &S, 16, NULL);

	
	ex = rsa_ver(&MM, &S, &E, &N);
	mpi_write_file("         верификация данных: ", &MM, 16, NULL);


    mpi_free( &E ); mpi_free( &P ); mpi_free( &Q ); mpi_free( &N );
    mpi_free( &H ); mpi_free( &D ); mpi_free( &X ); mpi_free( &Y );
    mpi_free( &Z );

	mpi_free( &G ); mpi_free( &M ); mpi_free( &S ); mpi_free( &MM );

#if defined(_WIN32)
    printf( "  Press Enter to exit this program...\n" );
    fflush( stdout ); getchar();
#endif

    return( 0 );
}