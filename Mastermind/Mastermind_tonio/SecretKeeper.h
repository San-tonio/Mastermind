#pragma once

#include <iostream>
#include <vector>
#include <tuple>

class SecretKeeper
{
public:
	SecretKeeper(const int size, const int maxNumber);//The principal constructor for our game, creates its own key
	SecretKeeper(const int size, const int maxNumber, const int* const Key);//Allows to choose a key
	~SecretKeeper(void);
	std::tr1::tuple<int, int> Guess(const int *guessAttempt);

private:
	const int m_nSecretSizeKeeper;
	const int m_nMaxNumber;
	std::shared_ptr<int> m_Secret;
};

