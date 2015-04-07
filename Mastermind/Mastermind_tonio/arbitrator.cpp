#include "StdAfx.h"
#include "arbitrator.h"
#include "SecretKeeper.h"
#include "Guesser.h"
#include "ChaoticGuesser.h"//alternative to our guesser!

arbitrator::arbitrator(const int secretSize, const int maxNumber, const int numberGames, const int nChaos, const std::string & filename):
	m_filename(filename),
	m_nSecretSize(secretSize),
	m_nMaxNumber(maxNumber),
	m_nNumberGames(numberGames),
	m_nChaos(nChaos),
	m_nGamesPlayed(0)
{
	m_bUsingFile = !filename.empty();

	if(m_bUsingFile)
	{
		m_streamKeys.open(m_filename, std::ios::in);
		if(!m_streamKeys)
		{
			printf("ERROR No file to read keys from, NOT playing from a file instead\n");//todo force crash?
			m_bUsingFile = false;
		}
		else
			GetNumberGamesFromFile();
	}
	
}

//Todo improvement: check that the file respects the MaxNumber parameter given
void arbitrator::GetNumberGamesFromFile()
{
	int numberKeys = 0;
	int keysize = -1;
	while(!m_streamKeys.eof())
	{
		std::string currentLine;
		if(getline(m_streamKeys,currentLine) && !currentLine.empty())
		{
			//std::cout<<currentLine << " was the line and the size is: " << currentLine.size() ;//debug
			numberKeys++;
			if(keysize == -1)
				keysize = currentLine.size();
			else if(keysize != currentLine.size())
			{
				printf("ERROR not all keys have the same length\n");//todo improve it by having dynamic key size
			}
		}
	}
	
	m_streamKeys.clear();
	m_streamKeys.seekg(0, std::ios::beg);

	//No keys detected in that file
	if(numberKeys<1)
	{
		printf("ERROR no keys detected in file\n");
	}

	m_nSecretSize = keysize;
	m_nNumberGames = numberKeys;
	printf("Debug from file: %d keys detected of size %d\n", m_nNumberGames, m_nSecretSize);
}


arbitrator::~arbitrator(void)
{
	if(m_bUsingFile)
		m_streamKeys.close();
}

void arbitrator::manageGame()
{
	std::vector<int> winScores;

	double tTotal=0;
	time_t tBegin, tEnd;

	//from the file or from our default value we know the number of games to play
	for(int i=0;i<m_nNumberGames;i++)
	{
		printf("\n ---------Game number %d --------\n",i);

		std::shared_ptr<SecretKeeper> graveyard;//std::unique_ptr

		int* currentKey= m_bUsingFile?new int[m_nSecretSize]:NULL;
		//instantiateKeyKeeper(graveyard, currentKey);//not clean for inheritance yet
		{
			/////secretKeeper: let it create a key
			if(!m_bUsingFile)
			{
				graveyard.reset(new SecretKeeper(m_nSecretSize, m_nMaxNumber));
			}
			/////secretKeep: read from file, get the next key!
			else
			{
				bool foundAKey = false;
				while(!m_streamKeys.eof() && !foundAKey)
				{
					std::string currentLine;
					if(getline(m_streamKeys,currentLine) && !currentLine.empty())
					{
						std::cout << currentLine << " detected has next key from file." << std::endl; ;//debug
						foundAKey = true;
						int value = atoi(currentLine.c_str());
						int decomposed = value;
						for(int i=0;i<m_nSecretSize;i++)
						{
							int digit = decomposed %10;
							decomposed = decomposed / 10;
							//std::cout << digit << std::endl;//debug
							currentKey[i]=digit;
						}
					}
				}
				graveyard.reset(new SecretKeeper(m_nSecretSize, m_nMaxNumber, currentKey));
			}
		}

		//TODO Urgent: improve that mess:

		////////////////guesser OPTION 1
		//This works fine, average 10rounds and 0.44s per round!
		//std::unique_ptr<Guesser> oracle;
		//oracle.reset(new Guesser(m_nSecretSize, m_nMaxNumber));

		////////////////guesser OPTION 2 
		//This works awesome, average 7rounds but 55s per round!
		std::unique_ptr<ChaoticGuesser> oracle;
		oracle.reset(new ChaoticGuesser(m_nSecretSize, m_nMaxNumber, m_nChaos));

		std::vector<std::tr1::tuple<int, int>> arrayRB;//to improve chaos algorithm

		const int maxTurn = 20;//for a game with a key of size 4, this is usually between 8and12
		int nCurrentTurn = 0;
		bool found = false;

		time(&tBegin);

		//Will try to let the player guess for 20 turns
		while( !found && nCurrentTurn < maxTurn)
		{
			int resultR,resultB;
			std::tie(resultR, resultB)= graveyard->Guess(oracle->Try());//auto result 
			oracle->Feedback(resultR, resultB);

			arrayRB.push_back(std::make_tuple(resultR,resultB));

			nCurrentTurn++;
			if(resultR==m_nSecretSize)
				found=true;
		}

		time(&tEnd);
		tTotal+=difftime(tEnd,tBegin);

		if(found)
		{
			winScores.push_back(nCurrentTurn);

			std::ofstream myfile;
			myfile.open ("mastermind_stats.txt", std::ios::app);
			myfile << "RB for " << nCurrentTurn << " turns\n";
			for(auto it=arrayRB.begin();it!=arrayRB.end();it++)
			{
				myfile << std::get<0>(*it) << " " << std::get<1>(*it) << "\n";
			}

			myfile.close();
		}
		printf("You %s , %d turns\n", found?"Won":"Lost", nCurrentTurn);

		delete[] currentKey;
	}

	printf("\n You played %d times, won %d times, average play time %f and your scores were:\n",m_nNumberGames, winScores.size(),tTotal/m_nNumberGames);
	for(auto it=winScores.begin();it!=winScores.end();it++)
	{
		std::cout << *it << " " ;
	}
	std::cout << std::endl;

}
