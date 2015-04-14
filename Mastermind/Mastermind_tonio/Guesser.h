#pragma once
#include <iostream>
#include <vector>

class Guesser
{
public:
	Guesser(const int size, const int maxNumber);
	~Guesser(void);
	int* Try();
	void Feedback(int nR, int nB);

private:
	const int m_nSecretSize;//number of unit in the secret
	const int m_nMaxNumber;//maximum value that a unit in the secret can value
	std::vector<int*> m_tryArrays;
	std::vector<int*> m_resultArrays;//R,B

	int* m_rightlyPlaced;//R
	std::vector<int> m_toPlace;//B

	int m_nRightlytPlaced;
	bool m_bDebug;
	bool m_bFeedback;

	int RplusB(int n);
	int numberColorsFound();
	void debugArrays(int*, int taille = -1);
	bool hasNeverBeenInColumn(int value, int column);

	void findSpot(int value, int* currentTryArray, int currentInteger);
	bool alreadyTried(int*currentTryArray);
	bool cachedGuessFits(int*currentTryArray);
};

