// Mastermind_tonio.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "arbitrator.h"
#include <stdlib.h> //for rand

int _tmain(int argc, _TCHAR* argv[])
{
	//Default value for the games, may be overwritten if there is a file with keys
	int defaultSecretSize = 6;//6 for our contest
	int defaultMaxNumber = 9;//Secret composed by integers from 0 to 9. for our contest
	int defaultNumberGames = 10;//We will play the game this number of times and give the scores at the end
	int chaosNumber = 3;//When solving with the chaotic guesser, gives the number of random iterations at the begining.

	std::string filename("PlayFromMe.txt");
	std::unique_ptr<arbitrator> trator;
	trator.reset(new arbitrator(defaultSecretSize, defaultMaxNumber, defaultNumberGames, chaosNumber, filename));
	
	//Initialize the Seed only once
	srand(42);

	trator->manageGame();

	system("pause");//windows
	//system("read");//for MM on mac

	return 0;
}

