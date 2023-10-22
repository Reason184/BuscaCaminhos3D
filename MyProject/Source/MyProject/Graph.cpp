// Fill out your copyright notice in the Description page of Project Settings.


#include "Graph.h"
#include <unordered_map>
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include <queue>


#pragma warning(disable: 4668)
Graph g_graph;

Graph::Graph()
{

}

AActor* Graph::generateGraph(AActor* fromComponent, UWorld* world, float length)
{

	if(!fromComponent) {
		return nullptr;
	}

	FVector start = fromComponent->GetActorLocation();


	for(auto const& OffSet : CubeOffsets) {

		float OffSetX = OffSet.X;
		float OffSetY = OffSet.Y;
		float OffSetZ = OffSet.Z;

		FVector forwardVector(OffSetX, OffSetY, OffSetZ);
		float dynamicLength = (OffSetX != 0 && OffSetY != 0) ? length * 1.f : length;


		if(OffSetX == 0 && OffSetY == 0 && OffSetZ == 0) {
			continue;
		}

		FVector end = start + (forwardVector * dynamicLength);
		FCollisionQueryParams params;
		params.AddIgnoredActor(fromComponent);
		//DrawDebugLine(world, start, end, FColor::Green, true, 1, 0, 2);
		FHitResult outhit;
		bool isHit = world->LineTraceSingleByChannel(outhit, start, end, ECC_Visibility, params);
		//UE_LOG(LogTemp, Warning, TEXT("fromComponent: %s"), *fromComponent->GetName());

		//UE_LOG(LogTemp, Warning, TEXT("ForwardVec: (%f, %f, %f)"), forwardVector.X, forwardVector.Y, forwardVector.Z);


		if(isHit) { // The ray hit something

			AActor* ref = outhit.GetActor();
			FString name = ref->GetName();
			//UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *name);
			FVector endHit = start + (forwardVector * outhit.Distance);

			if(ref && ref->ActorHasTag("node")) {// The ray hit another node
				mapGraph[fromComponent].push_back(ref);
			}

		}
		
	}


	return nullptr;
}


AActor* Graph::TraceLines(AActor* fromComponent, UWorld* world, float length)
{

	if(!fromComponent) {
		return nullptr;
	}

	FVector start = fromComponent->GetActorLocation();

	for(auto const& OffSet : CubeOffsets) {

		float OffSetX = OffSet.X;
		float OffSetY = OffSet.Y;
		float OffSetZ = OffSet.Z;

		FVector forwardVector(OffSetX, OffSetY, OffSetZ);
		float dynamicLength = (OffSetX != 0 && OffSetY != 0) ? length * 1.f : length;


		if(OffSetX == 0 && OffSetY == 0 && OffSetZ == 0) {
			continue;
		}

		FVector end = start + (forwardVector * dynamicLength);
		FCollisionQueryParams params;
		params.AddIgnoredActor(fromComponent);
		//DrawDebugLine(world, start, end, FColor::Green, true, 1, 0, 2);
		FHitResult outhit;
		bool isHit = world->LineTraceSingleByChannel(outhit, start, end, ECC_Visibility, params);
		//UE_LOG(LogTemp, Warning, TEXT("fromComponent: %s"), *fromComponent->GetName());

		//UE_LOG(LogTemp, Warning, TEXT("ForwardVec: (%f, %f, %f)"), forwardVector.X, forwardVector.Y, forwardVector.Z);


		if(isHit) { // The ray hit something

			AActor* ref = outhit.GetActor();
			FString name = ref->GetName();
			FVector endHit = start + (forwardVector * outhit.Distance);

			if(ref && ref->ActorHasTag("node")) {// The ray hit another node

				DrawDebugLine(world, start, end, FColor::Green, false, 10, 0, 2);

			} else { // O raio bateu em algo que não é um nó no grafo(cubo)
				//DrawDebugLine(world, start, end, FColor::Blue, false, 10, 0, 2);
			}
			//ref->Destroy();
		} else { // O raio não bateu em nada
			//DrawDebugLine(world, start, end, FColor::Red, false, 10, 0, 2);
		}
	}


	return nullptr;
}

void Graph::changeNodeColor(const FVector& nodePosition, FLinearColor color, AActor* hit)
{

	AActor* node = getNodeFromPos(nodePosition);
	if(!node)
		node = hit;
	
	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(node->GetComponentByClass(UStaticMeshComponent::StaticClass()));

	if(StaticMeshComponent) {

		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(StaticMeshComponent->GetMaterial(0), node);

		if(DynamicMaterial) {

			FLinearColor NewBaseColor = FLinearColor::Red;

			DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), color);

			StaticMeshComponent->SetMaterial(0, DynamicMaterial);

		}
	}

	

		
}

AActor* Graph::getNodeFromPos(const FVector& nodePosition)
{

	AActor** result = nodes.Find(nodePosition);

	if(result) 
		return nodes[nodePosition];
	 else 
		return nullptr;
	
}


bool Graph::simpleBFS(const FVector& fromPos, const FVector& toPos, TMap<FVector, FVector>& pred)
{


	if(fromPos == toPos) {
		return true;
	}

	FVector currentPosition, newPosition;
	std::list<FVector> queue;
	queue.push_back(fromPos);

	TMap<FVector, bool> src_visited;
	src_visited.Add(TTuple<FVector, bool>(fromPos, true));

	int numberOfNodes = 0;

	while(!queue.empty()) {

		currentPosition = queue.front();

		queue.pop_front();

		AActor* currentActor = nodes[currentPosition];
		for(const auto& actor : mapGraph[currentActor]) {

			newPosition = actor->GetActorLocation();

			auto nodeVisited = src_visited.Contains(newPosition);
			if(!nodeVisited) { // o nó na posição "newPosition" ainda não foi visitado
				numberOfNodes += 1;

				src_visited.Add(TTuple<FVector, bool>(newPosition, true));
				pred.Add(TTuple<FVector, FVector>(newPosition, currentPosition));
				queue.push_back(newPosition);

				if(newPosition == toPos) {
					GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Blue, FString::Printf(TEXT("Number of nodes visited: %i"), numberOfNodes), false);
					return true;
				}
			}
		}
	}
	return false;
}

void Graph::checkIntersection(graphVars* graphVariables, const FVector& pos, bool intersection)
{
	graphVariables->m_locking.lock();
	graphVariables->locking += 1;

	// Interseção encontrada, necessário guardar ela para ser usada na reconstrução do caminho
	if(intersection) {
		//UE_LOG(LogTemp, Warning, TEXT("Intersection found at: %i %i %i"), pos.X, pos.Y, pos.Z);
		graphVariables->intersection = pos;
	}

	if(graphVariables->locking == 2) {
		graphVariables->m_mainThread.unlock();
		graphVariables->condV.notify_all();
	}
	graphVariables->m_locking.unlock();
}




std::mutex mtx;
std::condition_variable cv;
int counter = 0;

void synchronized_start()
{
	std::unique_lock<std::mutex> lock(mtx);
	counter++;
	if(counter == 2) {
		cv.notify_all();
		//counter = 0;
	} else {
		cv.wait(lock, [] { return counter == 2; });
	}
}

void Graph::bidirectionalBFS(const FVector& fromPos, const FVector& toPos, graphVars* graphVariables)
{

	counter = 0;

	graphVariables->reset();

	// Cria threads separadas para executar as funções directedBFS e directedBFS2 usando a instância g_graph
	std::thread thread1(&Graph::directedBFS, &g_graph, fromPos, graphVariables, toPos);
	thread1.detach();
	std::thread thread2(&Graph::directedBFS2, &g_graph, toPos, graphVariables, fromPos);
	thread2.detach();


}

struct DistanceCompare
{
	FVector fromPos;

	DistanceCompare(const FVector& fromPos) : fromPos(fromPos) {}

	bool operator()(const FVector& a, const FVector& b) const
	{
		float distA = FVector::Dist(fromPos, a);
		float distB = FVector::Dist(fromPos, b);
		return distA >= distB;
	}
};

struct PathNode
{
	FVector position;
	float pathCost;

	PathNode(const FVector& position, float pathCost) : position(position), pathCost(pathCost) {}
};

struct PathNodeCompare
{
	bool operator()(const PathNode& a, const PathNode& b) const
	{
		return a.pathCost > b.pathCost;
	}
};

bool Graph::directedBFS(const FVector& fromPos, graphVars* graphVariables, const FVector& goal)
{
	synchronized_start();

	FVector currentPosition, newPosition;

	std::priority_queue<PathNode, std::vector<PathNode>, PathNodeCompare> queue;
	queue.push(PathNode(fromPos, 0.0f));

	graphVariables->m_firstVisited.lock();
	graphVariables->firstVisited.Add(fromPos, true);
	graphVariables->m_firstVisited.unlock();

	while(!queue.empty()) {
		PathNode currentPathNode = queue.top();
		currentPosition = currentPathNode.position;
		queue.pop();

		AActor* currentActor = nodes[currentPosition];
		for(const auto& actor : mapGraph[currentActor]) {
			newPosition = actor->GetActorLocation();
			auto search = graphVariables->firstVisited.Contains(newPosition);
			if(!search) {

				graphVariables->m_nodesVisited.lock();
				graphVariables->nodesVisited += 1;
				graphVariables->m_nodesVisited.unlock();

				graphVariables->m_firstVisited.lock();
				graphVariables->firstVisited.Add(newPosition, true);
				graphVariables->m_firstVisited.unlock();

				graphVariables->firstPred.Add(newPosition, currentPosition);

				float newCost = currentPathNode.pathCost + FVector::Dist(currentPosition, newPosition);
				queue.push(PathNode(newPosition, newCost));

				if(graphVariables->intersection != FVector(0, 0, 0)) { // Interseção encontrada pela outra thread, então é necessário parar essa.
					checkIntersection(graphVariables, newPosition, false);
					return true;
				}

				graphVariables->m_secondVisited.lock();
				auto otherThreadVisitedNodes = graphVariables->secondVisited.Contains(newPosition);
				graphVariables->m_secondVisited.unlock();

				if(otherThreadVisitedNodes) { // Interseção encontrada por essa thread, necessário avisar a outra para não continuar expandindo.
					checkIntersection(graphVariables, newPosition, true);
					return true;
				}
			}
		}
	}
	checkIntersection(graphVariables, newPosition, false);
	return false;
}


bool Graph::directedBFS2(const FVector& fromPos, graphVars* graphVariables, const FVector& goal)
{


	synchronized_start();

	FVector currentPosition, newPosition;
	DistanceCompare cmp(fromPos);
	std::priority_queue<PathNode, std::vector<PathNode>, PathNodeCompare> queue;
	queue.push(PathNode(fromPos, 0.0f));

	graphVariables->m_secondVisited.lock();
	graphVariables->secondVisited.Add(fromPos, true);
	graphVariables->m_secondVisited.unlock();


	while(!queue.empty()) {


		PathNode currentPathNode = queue.top();
		currentPosition = currentPathNode.position;
		queue.pop();

		AActor* currentActor = nodes[currentPosition];
		for(const auto& actor : mapGraph[currentActor]) {


			newPosition = actor->GetActorLocation();
			auto search = graphVariables->secondVisited.Contains(newPosition);
			if(!search) {

				graphVariables->m_nodesVisited.lock();
				graphVariables->nodesVisited += 1;
				graphVariables->m_nodesVisited.unlock();

				graphVariables->m_secondVisited.lock();

				graphVariables->secondVisited.Add(newPosition, true);
				graphVariables->m_secondVisited.unlock();

				graphVariables->secondPred.Add(newPosition, currentPosition);

				float newCost = currentPathNode.pathCost + FVector::Dist(currentPosition, newPosition);
				queue.push(PathNode(newPosition, newCost));

				if(graphVariables->intersection != FVector(0, 0, 0)) { // Interseção encontrada pela outra thread, então é necessário parar essa.
					checkIntersection(graphVariables, newPosition, false);
					return true;
				}

				graphVariables->m_firstVisited.lock();
				auto otherThreadVisitedNodes = graphVariables->firstVisited.Contains(newPosition);
				graphVariables->m_firstVisited.unlock();

				if(otherThreadVisitedNodes) { // Interseção encontrada por essa thread, necessário avisar a outra para não continuar expandindo.
					checkIntersection(graphVariables, newPosition, true);
					return true;
				}
			}
		}
	}
	checkIntersection(graphVariables, newPosition, false);
	return false;
}

Graph::~Graph()
{

}
