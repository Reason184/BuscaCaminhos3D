// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <vector>
#include "DrawDebugHelpers.h"
#include "MyProject.h"
#include "mutex"
#include <unordered_map>

/**
 * 
 */

class graphVars
{
public:

	graphVars()
	{
		locking = 0;
		nodesVisited = 0;
		intersection = FVector(0, 0, 0);

	};

	void reset() {
		locking = 0;
		nodesVisited = 0;
		intersection = FVector(0, 0, 0);
		firstPred.Empty();
		secondPred.Empty();
		firstVisited.Empty();
		secondVisited.Empty();
	}

	TMap<FVector, FVector> firstPred, secondPred;

	TMap<FVector, bool> firstVisited, secondVisited;

	FVector intersection;

	int locking, nodesVisited;
	std::mutex m_firstVisited, m_secondVisited, m_locking, m_mainThread, m_nodesVisited;



	std::condition_variable condV;


};

static std::vector<FVector> CubeOffsets{

	//This will first generate the lines coming from the center of each face
	FVector(0, -1, 0),
	FVector(-1, 0, 0),
	FVector(0, 1, 0),
	FVector(1, 0, 0),
	FVector(-1, 0, 1),
	FVector(0, -1, 1),
	FVector(0, 0, 1),
	FVector(1, 0, 1),
	FVector(0, 1, 1),
	FVector(-1, 0, -1),
	FVector(0, -1, -1),
	FVector(0, 0, -1),
	FVector(0, 1, -1),
	FVector(1, 0, -1),

	//The remaining lines are in the edges of each face
	FVector(-1, -1, -1),
	FVector(-1, -1, 0),
	FVector(-1, -1, 1),
	FVector(-1, 1, -1),
	FVector(-1, 1, 0),
	FVector(-1, 1, 1),
	FVector(1, -1, -1),
	FVector(1, -1, 0),
	FVector(1, -1, 1),
	FVector(1, 1, -1),
	FVector(1, 1, 0),
	FVector(1, 1, 1)

};

class MYPROJECT_API Graph
{
public:



	TMap<FVector, AActor*> nodes;

	std::unordered_map<AActor*, std::vector<AActor*>> mapGraph;

	TMap<int, int> newnodes;

	AActor* TraceLines(AActor* fromComponent, UWorld* world, float length);

	void changeNodeColor(const FVector& nodePosition, FLinearColor color, AActor* hit = nullptr);

	AActor* getNodeFromPos(const FVector& nodePosition);


	Graph();
	AActor* generateGraph(AActor* fromComponent, UWorld* world, float length);
	bool simpleBFS(const FVector& fromPos, const FVector& toPos, TMap<FVector, FVector>& pred);
	void checkIntersection(graphVars* graphVariables, const FVector& pos, bool intersection);
	bool directedBFS(const FVector& fromPos, graphVars* graphVariables, const FVector& goal);
	bool directedBFS2(const FVector& fromPos, graphVars* graphVariables, const FVector& goal);
	void bidirectionalBFS(const FVector& fromPos, const FVector& toPos, graphVars* graphVariables);

	~Graph();
};
