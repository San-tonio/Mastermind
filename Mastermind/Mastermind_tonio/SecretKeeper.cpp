#include "StdAfx.h"
#include "SecretKeeper.h"

SecretKeeper::SecretKeeper(const int secretSize, const int maxNumber):
	m_nSecretSizeKeeper(secretSize),
	m_nMaxNumber(maxNumber)
{
	m_Secret.reset(new int[secretSize]);

	//Get the code from the user or generate it randomly?
	//Random generation
	std::cout << "Debug the code you'll be looking for is ";//debug cheat
	for(int i = 0; i < secretSize; i++)
	{
		m_Secret.get()[i] = rand() % (maxNumber+1);

		std::cout << m_Secret.get()[i];//debug cheat
	}
	std::cout << std::endl;//debug cheat
}

SecretKeeper::SecretKeeper(const int secretSize, const int maxNumber, const int* const Key):
	m_nSecretSizeKeeper(secretSize),
	m_nMaxNumber(maxNumber)
{
	if(Key==NULL)
	{
		printf("ERROR in SecretKeeper, key not supposed to be null\n");
	}

	m_Secret.reset(new int[secretSize]);

	for(int i = 0; i < secretSize; i++)
	{
		m_Secret.get()[i] = Key[i];
	}
}

SecretKeeper::~SecretKeeper(void)
{
}

std::tr1::tuple<int, int> SecretKeeper::Guess(const int *guessAttempt)//[9])
{
	if(guessAttempt == NULL)
	{
		printf("Empty attempt, return 0,0\n");
		return std::make_tuple(0,0);
	}

	int nR=0;//number of elements rightly placed
	int nB=0;//number of elements misplaced
	std::vector<int> potentialyMisplacedGuess;
	std::vector<int> potentialyMisplacedSecret;

	//Find the rightly placed integers
	for(int i=0; i<m_nSecretSizeKeeper; i++)
	{
		if(guessAttempt[i] == m_Secret.get()[i])
		{
			nR++;
		}
		else
		{
			potentialyMisplacedGuess.push_back(guessAttempt[i]);
			potentialyMisplacedSecret.push_back(m_Secret.get()[i]);
		}
	}

	//Find the misplaced numbers
	//For each number of the guess which isn't placed correctly
	for(int i=0;i<(int)potentialyMisplacedGuess.size();i++)
	{
		//Check if there is such a misplaced(too) integer in the secret (and remove it from the list)
		int j=0;
		//parse the list until we find it or not!
		while(j<(int)potentialyMisplacedSecret.size() && potentialyMisplacedSecret[j] != potentialyMisplacedGuess[i])
		{
			j++;
		}

		//if j is in the bound, we found one!
		if(j<(int)potentialyMisplacedSecret.size())
		{
			nB++;
			potentialyMisplacedSecret[j] = -1;//give it a dumb value so we don't take it into consideration on other comparisons. (could also remove it from list)
		}
	}

	return std::make_tuple(nR,nB);
}