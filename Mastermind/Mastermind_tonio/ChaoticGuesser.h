#pragma once
#include <iostream>
#include <vector>

class ChaoticGuesser
{
public:
	ChaoticGuesser(const int size, const int maxNumber, const int nChaos);
	~ChaoticGuesser(void);
	int* Try();
	void Feedback(int nR, int nB);

private:
	const int m_nSecretSize;
	const int m_nMaxNumber;
	const int m_nChaos;//Number of iterations for which we'll try randomly
	int m_nCurrentNumber;
	std::vector<int*> m_tryArrays;
	std::vector<int*> m_resultArrays;
	
	void debugArrays(int*, int taille = -1);
	bool hasNeverBeenInColumn(int value, int column);
	bool alreadyTried(int*currentTryArray);
	bool cachedGuessFits(int*currentTryArray);

	//we don't use that logic here
	//int* m_rightlyPlaced;//R
	//std::vector<int> m_toPlace;//B
	//int m_nRightlytPlaced;
	//int RplusB(int n);
	//int numberColorsFound();
	//void findSpot(int value, int* currentTryArray, int currentInteger);
};
