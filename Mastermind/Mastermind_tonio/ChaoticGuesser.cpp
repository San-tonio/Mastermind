#include "StdAfx.h"
#include "ChaoticGuesser.h"
#include <algorithm>    // std::sort
#include "SecretKeeper.h"
#include <cmath>
#include <time.h>
#include <limits>//for debug
#include <algorithm>    // std::sort

int fact(int n)
{
	return (n==0 || n==1)?1:n*fact(n-1);
}

ChaoticGuesser::ChaoticGuesser(const int size, const int maxNumber, const int nChaos):
	m_nSecretSize(size),
	m_nMaxNumber(maxNumber),
	m_nChaos(nChaos),
	m_nCurrentNumber(0)
{
	//Check that we won't be in trouble regarding overflows
	if(std::numeric_limits<int>::max() < pow(10.0, m_nMaxNumber))
		printf("ERROR max int = %d\n",std::numeric_limits<int>::max());
}

ChaoticGuesser::~ChaoticGuesser(void)
{
	for(int i=0;i<(int)m_tryArrays.size();i++)
	{
		delete[] m_tryArrays[i];
	}

	for(int i=0;i<(int)m_resultArrays.size();i++)
	{
		delete[] m_resultArrays[i];
	}
}

int *ChaoticGuesser::Try(void)
{
	int* currentTry = new int[m_nSecretSize];
	int lastLine = (int)m_resultArrays.size()-1;//for optimization

	//first ? lines, try randomly
	if((int)m_tryArrays.size()<m_nChaos)//nchaos here //4=>8 turns on average; 3=>7.2 on average!; 0=>10,11; 1=>7,78; 2=>7,78
	{
		for(int i = 0; i < m_nSecretSize; i++)
		{
			currentTry[i] = rand() % (m_nMaxNumber+1);
		}
	}
	//Last try optimization
	else if(m_resultArrays[lastLine][0]+m_resultArrays[lastLine][1]==m_nSecretSize)
	{
		time_t tBegin,tEnd;
		time(&tBegin);

		std::vector<int> lastTryElements;
		for(int j=0;j<m_nSecretSize;j++)
		{
			lastTryElements.push_back(m_tryArrays[lastLine][j]);
		}
		std::sort(lastTryElements.begin(), lastTryElements.end());
		
		int index=0;
		for(auto it=lastTryElements.begin();it!=lastTryElements.end();it++)
		{
			currentTry[index]=*it;
			index++;
		}

		int numberPossibilities=fact(m_nSecretSize);
		int i=0;
		while( (alreadyTried(currentTry) || !cachedGuessFits(currentTry)) && i<numberPossibilities)
		{
			std::next_permutation(currentTry,currentTry+m_nSecretSize);
			i++;
		}
		if(i==numberPossibilities)
			printf("ERROR: detected that we tried all combinations already oO\n");

		time(&tEnd);
		printf("time to find a permutation = %f\n", difftime(tEnd, tBegin));//for stats
	}
	else
	{
		double numberPossibilities= pow(10.0, m_nSecretSize);
		time_t tBegin,tEnd;
		time(&tBegin);
		
		int i=m_nCurrentNumber;//we can start from the last number we tried
		do
		{
			//Extract the digits from the number, fills the array from the end
			int n=i;
			int index=m_nSecretSize-1;
			do {
				//integer to array
				int digit = n % 10;
				if(index<0 || index>=m_nSecretSize)
					printf("ERROR going out of bound!\n");
				currentTry[index]=digit;
				index--;
				n /= 10;
			} while (n > 0);

			//Finish completing the array
			for(int j = index; j >=0 ; j--)
			{
				currentTry[j] = 0;
			}

			i++;
		}
		while( (alreadyTried(currentTry) || !cachedGuessFits(currentTry)) && i<numberPossibilities);

		m_nCurrentNumber = i;//no need to retry all the numbers we already checked

		time(&tEnd);
		printf("time to find a solution = %f\n", difftime(tEnd, tBegin));//stats
	}
	
	debugArrays(currentTry);

	m_tryArrays.push_back(currentTry);
	return currentTry;
}

void ChaoticGuesser::debugArrays(int* myArray, int taille)
{
	if(taille==-1)
		taille=m_nSecretSize;

	if(myArray==NULL)
		return;

	for(int i=0; i<taille;i++)
	{
		std::cout << myArray[i] << " ";
		if(myArray[i]<0)//debug
			printf("WTF\n");
	}

	std::cout << std::endl;
}

bool ChaoticGuesser::cachedGuessFits(int*currentTryArray)
{
	//Avoid code duplication, we create our own guesser and assign it the value we think could be the key, just to compare the (R,B) results
	std::unique_ptr<SecretKeeper> subordinate;
	subordinate.reset(new SecretKeeper(m_nSecretSize, m_nMaxNumber, currentTryArray));

	//For each line of tryArray, determine if R,B is the same as it would be
	for(int ligne=0;ligne<(int)m_tryArrays.size();ligne++)
	{
		int nR=0;//number of elements rightly placed
		int nB=0;//number of elements misplaced
		std::tie(nR, nB)= subordinate->Guess(m_tryArrays[ligne]);//auto result 

		if(m_resultArrays[ligne][0]!=nR || m_resultArrays[ligne][1]!=nB)
		{
			//Chaos version: we don't want to see the shit load of output =D
			//printf("Debug currentTry try can't fit:");
			//debugArrays(currentTryArray);
			//printf("we would have (nR,nB)= %d,%d and we had %d,%d on line %d\n",nR,nB,m_resultArrays[ligne][0], m_resultArrays[ligne][1],ligne );
			return false;
		}
	}
	return true;
}

bool ChaoticGuesser::alreadyTried(int*currentTryArray)
{
	for(int i=0; i!=m_resultArrays.size(); i++)
	{
		bool areSame = true;
		int j=0;
		while(areSame && j<m_nSecretSize)
		{
			if(m_tryArrays[i][j]!=currentTryArray[j])
			{
				areSame = false;
			}
			j++;
		}
		if(areSame)
		{
			//printf("DEBUG we already tried that combination ");
			//debugArrays(currentTryArray);
			return true;
		}
	}
	return false;
}

void ChaoticGuesser::Feedback(int nR, int nB)
{
	int* result = new int[2];
	result[0] = nR;
	result[1] = nB;

	std::cout << nR << " " << nB << std::endl;

	const int currentValue = m_resultArrays.size();
	m_resultArrays.push_back(result);//currentValue parameter 

	//Now we don't compute the feedback! =D
}
