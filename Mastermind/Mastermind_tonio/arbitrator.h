#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <time.h>
#include <fstream>

class arbitrator
{
public:
	arbitrator(const int secretSize, const int maxNumber, const int numberGames, const int nChaos, const std::string & filename = "");
	~arbitrator(void);
	void manageGame();

private:
	int m_nSecretSize;
	int m_nMaxNumber;
	int m_nNumberGames;
	int m_nChaos;
	bool m_bUsingFile;
	std::string m_filename;//Are we going to read from a file or not? //Hypothesis: in a file, all the keys have the same length of 6 and respect our maxNumber above
	std::ifstream m_streamKeys;
	int m_nGamesPlayed;

	void GetNumberGamesFromFile();//way to overwrite m_nSecretSize m_nNumberGames with data from file
};

