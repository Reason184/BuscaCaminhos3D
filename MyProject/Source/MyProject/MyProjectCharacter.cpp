// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyProjectCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

#include "MyProject.h"
#include "enums.h"
#include <string>

#include "Blueprint/UserWidget.h"

#include "TextWidget.h"

#include "GameFramework/SpringArmComponent.h"


#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "HAL/PlatformProcess.h"
#include "HAL/ThreadSafeBool.h"
#include "Misc/ScopeLock.h"
FCriticalSection firstVisitedCriticalSection, secondVisitedCriticalSection;


extern Graph g_graph;



//////////////////////////////////////////////////////////////////////////
// AMyProjectCharacter

AMyProjectCharacter::AMyProjectCharacter() : graphVariables()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	PrimaryActorTick.bCanEverTick = true;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	if(IsValid(WidgetClass)) {
		TextWidget = Cast<UTextWidget>(CreateWidget(GetWorld(), WidgetClass));

		if(TextWidget) {
			TextWidget->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("ADDING TO VIEWPORT..."));
		}
	}


	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	g_graph = Graph();

	targetPosition = FVector(0, 0, 0);
	startPosition = FVector(0, 0, 0);
	endPosition = FVector(0, 0, 0);

	spaceDelta = 200;
	nodePath.Empty();

}


// Called every frame
void AMyProjectCharacter::Tick(float DeltaTime)
{
	//Super::Tick(DeltaTime);

	//FVector location = GetActorLocation();
	
	//location += FVector(1, 0, 10);
	
	//SetActorLocation(location);

}

void AMyProjectCharacter::GenerateGraph()
{
	float dist = spaceDelta;

	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, FString::Printf(TEXT("Generating graph...")), false);

	//g_graph.mapGraph.clear();


	for(auto& kv : g_graph.mapGraph) {
		kv.second.clear();
	}
	g_graph.mapGraph.clear();

	for(const auto& ref : g_graph.nodes) {

		// normal nodes

		//UE_LOG(LogTemp, Warning, TEXT("Generating Graph..."));
		g_graph.generateGraph(ref.Value, GetWorld(), dist);


	}
}

void AMyProjectCharacter::TraceLines()
{
	static TMap<int32, FString> FruitMap;
	float dist = spaceDelta;

	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, FString::Printf(TEXT("Tracing lines from nodes...")), false);

	
	for(const auto& ref : g_graph.nodes) {

		// normal nodes

		//UE_LOG(LogTemp, Warning, TEXT("Casting Rays..."));
		g_graph.TraceLines(ref.Value, GetWorld(), dist);


	}
}

void AMyProjectCharacter::StartSimpleBFS()
{

	FBenchmark t;
	TMap<FVector, FVector> pred;

	//std::unordered_map<uint64_t, uint64_t> newPred;



	if(startPosition == FVector(0,0,0) || endPosition == FVector(0, 0, 0)) {
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, FString::Printf(TEXT("Start or endposition not set")), false);
		return;

	}

	if(!g_graph.simpleBFS(startPosition, endPosition, pred)) {
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, FString::Printf(TEXT("No path found.")), false);
		return;
	}

	nodePath.Empty();


	//uint64_t currentHash = BFSHash(toPos);
	FVector currentPosition = endPosition;
	nodePath.Add(endPosition);

	//Loop the map until we find the destination
	// while(newPred.find(currentHash) != newPred.end()) {

	while(!pred[currentPosition].Equals(startPosition)) {

		nodePath.Emplace(pred[currentPosition]);
		currentPosition = pred[currentPosition];

	}
	nodePath.Emplace(startPosition);



	FVector lastPosition(0, 0, 0);
	for(auto const& pos: nodePath) {

		if(lastPosition != FVector(0, 0, 0)) {
			FVector start = lastPosition;
			FVector end = pos;
			DrawDebugLine(GetWorld(), start, end, FColor::Green, false, 10, 0, 2);
		}
		lastPosition = pos;
		g_graph.changeNodeColor(pos, FLinearColor::Red);
		//GEngine->AddOnScreenDebugMessage(-1, 6, FColor::Yellow, FString::Printf(TEXT("Location: (%f, %f, %f)"), pos.X, pos.Y, pos.Z), false);
	}
	
	float MovementSpeed = 3.f;
	MoveCharacterSmoothly(endPosition, MovementSpeed);

	
}


void AMyProjectCharacter::StartBidirectionalBFS()
{


	if(startPosition == FVector(0, 0, 0) || endPosition == FVector(0, 0, 0)) {
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, FString::Printf(TEXT("Start or endposition not set")), false);
		return;
	}

	graphVariables.reset();

	std::unique_lock<std::mutex> mainLock(graphVariables.m_mainThread);

	//lock the main mutex
	{
		FBenchmark t;
		g_graph.bidirectionalBFS(startPosition, endPosition, &graphVariables);
		graphVariables.condV.wait(mainLock);
	}


	if(graphVariables.intersection == FVector(0,0,0)) {
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, FString::Printf(TEXT("Intersection is null")), false);
		return;

	}


	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Blue, FString::Printf(TEXT("Number of nodes visited: %i"), graphVariables.nodesVisited), false);
	nodePath.Empty();



	FVector currentPosition = graphVariables.intersection;
	//GEngine->AddOnScreenDebugMessage(-1, 6, FColor::Red, FString::Printf(TEXT("Intersection: (%f, %f, %f)"), graphVariables.intersection.X, graphVariables.intersection.Y, graphVariables.intersection.Z), false);
	
	nodePath.Add(currentPosition);
	g_graph.changeNodeColor(currentPosition, FLinearColor::Red);

	while(!graphVariables.firstPred[currentPosition].Equals(startPosition)) {

		//GEngine->AddOnScreenDebugMessage(-1, 6, FColor::Yellow, FString::Printf(TEXT("Location: (%f, %f, %f)"), graphVariables.firstPred[currentPosition].X, graphVariables.firstPred[currentPosition].Y, graphVariables.firstPred[currentPosition].Z), false);
		nodePath.Emplace(graphVariables.firstPred[currentPosition]);
		g_graph.changeNodeColor(graphVariables.firstPred[currentPosition], FLinearColor::Yellow);
		currentPosition = graphVariables.firstPred[currentPosition];

	}

	nodePath.Add(startPosition);
	currentPosition = graphVariables.intersection;
	
	while(!graphVariables.secondPred[currentPosition].Equals(endPosition)) {

		//GEngine->AddOnScreenDebugMessage(-1, 6, FColor::Blue, FString::Printf(TEXT("Location: (%f, %f, %f)"), graphVariables.secondPred[currentPosition].X, graphVariables.secondPred[currentPosition].Y, graphVariables.secondPred[currentPosition].Z), false);
		nodePath.Insert(graphVariables.secondPred[currentPosition], 0);
		g_graph.changeNodeColor(graphVariables.secondPred[currentPosition], FLinearColor::Blue);
		currentPosition = graphVariables.secondPred[currentPosition];

	}
	
	nodePath.Insert(endPosition, 0);

	
	FVector lastPosition(0, 0, 0);
	for(auto const& pos : nodePath) {

		if(lastPosition != FVector(0, 0, 0)) {
			FVector start = lastPosition;
			FVector end = pos;
			//GEngine->AddOnScreenDebugMessage(-1, 6, FColor::Yellow, FString::Printf(TEXT("Location: (%f, %f, %f)"), pos.X, pos.Y, pos.Z), false);
			DrawDebugLine(GetWorld(), start, end, FColor::Green, false, 10, 0, 2);
		}
		lastPosition = pos;
		//g_graph.changeNodeColor(pos, FLinearColor::Red);
	}


	float MovementSpeed = 3.f;
	MoveCharacterSmoothly(endPosition, MovementSpeed);


}

void AMyProjectCharacter::SetStartPosition()
{

	

	FVector start = FollowCamera->GetComponentLocation();
	FVector forwardVector = FollowCamera->GetForwardVector();
	start = start + (forwardVector * CameraBoom->TargetArmLength);
	FVector end = start + (forwardVector * 600);
	FCollisionQueryParams params;
	params.AddIgnoredActor(GetOwner());

	//DrawDebugLine(GetWorld(), start, end, FColor::Green, true, 1, 0, 1);

	FHitResult outhit;
	bool isHit = GetWorld()->LineTraceSingleByChannel(outhit, start, end, ECC_Visibility, params);


	if(isHit) {
		AActor* ref = outhit.GetActor();
		if(ref && ref->ActorHasTag("node")) {// The ray hit another node


			FString name = ref->GetName();

			g_graph.changeNodeColor(ref->GetActorLocation(), FLinearColor::Yellow);

			GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, FString::Printf(TEXT("StartPosition: (%.2f, %.2f, %.2f)"), startPosition.X, startPosition.Y, startPosition.Z), false);

			startPosition = ref->GetActorLocation();
		}
		
		//ref->Destroy();
	}


}

void AMyProjectCharacter::SetEndPosition()
{

	FVector start = FollowCamera->GetComponentLocation();
	FVector forwardVector = FollowCamera->GetForwardVector();
	start = start + (forwardVector * CameraBoom->TargetArmLength);
	FVector end = start + (forwardVector * 600);
	FCollisionQueryParams params;
	params.AddIgnoredActor(GetOwner());

	FHitResult outhit;
	bool isHit = GetWorld()->LineTraceSingleByChannel(outhit, start, end, ECC_Visibility, params);


	if(isHit) {
		AActor* ref = outhit.GetActor();
		//	
		if(ref && ref->ActorHasTag("node")) { //O raio bateu em um nó(cubo) do grafo


			FString name = ref->GetName();

			g_graph.changeNodeColor(ref->GetActorLocation(), FLinearColor::Blue);
			endPosition = ref->GetActorLocation();
			GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, FString::Printf(TEXT("EndPosition: (%.2f, %.2f, %.2f)"), endPosition.X, endPosition.Y, endPosition.Z), false);

		}

		//ref->Destroy();
	}


}


void AMyProjectCharacter::GetActorPosition()
{



	FVector start = FollowCamera->GetComponentLocation();
	FVector forwardVector = FollowCamera->GetForwardVector();
	start = start + (forwardVector * CameraBoom->TargetArmLength);
	FVector end = start + (forwardVector * 600);
	FCollisionQueryParams params;
	params.AddIgnoredActor(GetOwner());

	//DrawDebugLine(GetWorld(), start, end, FColor::Green, true, 1, 0, 1);

	FHitResult outhit;
	bool isHit = GetWorld()->LineTraceSingleByChannel(outhit, start, end, ECC_Visibility, params);


	if(isHit) {
		AActor* ref = outhit.GetActor();
		//	
		if(ref && ref->ActorHasTag("node")) {//O raio bateu em um nó(cubo) do grafo


			FString name = ref->GetName();

			GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, FString::Printf(TEXT("Location: (%f, %f, %f)"), ref->GetActorLocation().X, ref->GetActorLocation().Y, ref->GetActorLocation().Z), false);
		}

		//ref->Destroy();
	}


}


void AMyProjectCharacter::SetColorToBlue()
{

	FVector start = FollowCamera->GetComponentLocation();
	FVector forwardVector = FollowCamera->GetForwardVector();
	start = start + (forwardVector * CameraBoom->TargetArmLength);
	FVector end = start + (forwardVector * 600);

	FCollisionQueryParams params;
	params.AddIgnoredActor(GetOwner());

	FHitResult outhit;
	bool isHit = GetWorld()->LineTraceSingleByChannel(outhit, start, end, ECC_Visibility, params);

	if(isHit) {
		AActor* ref = outhit.GetActor();
		if(ref && ref->ActorHasTag("node")) {//O raio bateu em um nó(cubo) do grafo
			g_graph.changeNodeColor(ref->GetActorLocation(), FLinearColor::Blue);
		}
	}
}

void AMyProjectCharacter::SetColorToOrange()
{

	FVector start = FollowCamera->GetComponentLocation();
	FVector forwardVector = FollowCamera->GetForwardVector();
	start = start + (forwardVector * CameraBoom->TargetArmLength);
	FVector end = start + (forwardVector * 600);

	FCollisionQueryParams params;
	params.AddIgnoredActor(GetOwner());

	FHitResult outhit;
	bool isHit = GetWorld()->LineTraceSingleByChannel(outhit, start, end, ECC_Visibility, params);

	if(isHit) {
		AActor* ref = outhit.GetActor();
		if(ref && ref->ActorHasTag("node")) {//O raio bateu em um nó(cubo) do grafo
			g_graph.changeNodeColor(ref->GetActorLocation(), FLinearColor(1.0f, 0.5f, 0.0f, 1.0f));
		}
	}
}

void AMyProjectCharacter::SetColorToPurple()
{

	FVector start = FollowCamera->GetComponentLocation();
	FVector forwardVector = FollowCamera->GetForwardVector();
	start = start + (forwardVector * CameraBoom->TargetArmLength);
	FVector end = start + (forwardVector * 600);

	FCollisionQueryParams params;
	params.AddIgnoredActor(GetOwner());

	FHitResult outhit;
	bool isHit = GetWorld()->LineTraceSingleByChannel(outhit, start, end, ECC_Visibility, params);

	if(isHit) {
		AActor* ref = outhit.GetActor();
		if(ref && ref->ActorHasTag("node")) {//O raio bateu em um nó(cubo) do grafo
			g_graph.changeNodeColor(ref->GetActorLocation(), FLinearColor(0.5f, 0.0f, 0.5f, 1.0f));
		}
	}
}

void AMyProjectCharacter::SetColorToBrown()
{

	FVector start = FollowCamera->GetComponentLocation();
	FVector forwardVector = FollowCamera->GetForwardVector();
	start = start + (forwardVector * CameraBoom->TargetArmLength);
	FVector end = start + (forwardVector * 600);

	FCollisionQueryParams params;
	params.AddIgnoredActor(GetOwner());

	FHitResult outhit;
	bool isHit = GetWorld()->LineTraceSingleByChannel(outhit, start, end, ECC_Visibility, params);

	if(isHit) {
		AActor* ref = outhit.GetActor();
		if(ref && ref->ActorHasTag("node")) {//O raio bateu em um nó(cubo) do grafo
			g_graph.changeNodeColor(ref->GetActorLocation(), FLinearColor(0.4f, 0.26f, 0.13f, 1.0f));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Input


void AMyProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{

	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("SetStartPosition", IE_Pressed, this, &AMyProjectCharacter::SetStartPosition);
	PlayerInputComponent->BindAction("SetEndPosition", IE_Pressed, this, &AMyProjectCharacter::SetEndPosition);

	PlayerInputComponent->BindAction("GetActorPosition", IE_Pressed, this, &AMyProjectCharacter::GetActorPosition);


	PlayerInputComponent->BindAction("setColorToBlue", IE_Pressed, this, &AMyProjectCharacter::SetColorToBlue);
	PlayerInputComponent->BindAction("setColorToOrange", IE_Pressed, this, &AMyProjectCharacter::SetColorToOrange);
	PlayerInputComponent->BindAction("setColorToPurple", IE_Pressed, this, &AMyProjectCharacter::SetColorToPurple);
	PlayerInputComponent->BindAction("setColorToBrown", IE_Pressed, this, &AMyProjectCharacter::SetColorToBrown);

	PlayerInputComponent->BindAction("TraceLines", IE_Released, this, &AMyProjectCharacter::TraceLines);

	PlayerInputComponent->BindAction("GenerateGraph", IE_Released, this, &AMyProjectCharacter::GenerateGraph);

	PlayerInputComponent->BindAction("IncreasenUmberOfNodes", IE_Released, this, &AMyProjectCharacter::IncreaseNumberOfNodes);
	PlayerInputComponent->BindAction("DecreaseNumberOfNodes", IE_Released, this, &AMyProjectCharacter::DecreaseNumberOfNodes);

	PlayerInputComponent->BindAction("Reset", IE_Released, this, &AMyProjectCharacter::Reset);


	PlayerInputComponent->BindAction("StartSimpleBFS", IE_Pressed, this, &AMyProjectCharacter::StartSimpleBFS);
	PlayerInputComponent->BindAction("StartBidirectionalBFS", IE_Pressed, this, &AMyProjectCharacter::StartBidirectionalBFS);

	PlayerInputComponent->BindAction("StartCode", IE_Released, this, &AMyProjectCharacter::SpawnActor);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyProjectCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMyProjectCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMyProjectCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMyProjectCharacter::OnResetVR);
}


void AMyProjectCharacter::OnResetVR()
{
	// If MyProject is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in MyProject.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMyProjectCharacter::SpawnActor()
{
	
	const FRotator rotation = GetActorRotation();

	Reset();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AActor::StaticClass(), "nodeBlock", FoundActors);

	for(float x = -1800.f; x <= 600; x += spaceDelta) {
		for(float y = -4000.f; y <= 0; y += spaceDelta) {
			for(float z = 200.f; z <= 600; z += spaceDelta) {

				//Spawn Node
				FVector location(x, y, z);
				AActor* nodeSpawned = GetWorld()->SpawnActor<AActor>(ActorToSpawn, location, FRotator(0, 0, 0));

				FVector nodeOrigin, nodeExtent;

				float transformScale = spaceDelta / 200;
				nodeSpawned->SetActorScale3D(FVector(transformScale, transformScale, transformScale));

				nodeSpawned->GetActorBounds(false, nodeOrigin, nodeExtent);

				FBox nodeBounds(nodeOrigin - nodeExtent, nodeOrigin + nodeExtent);

				bool isCollidingWithOtherActor = false;
				for(auto const blockedItem : FoundActors) {

					FVector actorOrigin, actorExtent;

					blockedItem->GetActorBounds(false, actorOrigin, actorExtent);

					actorExtent = actorExtent + nodeExtent;
					FBox actorBounds(actorOrigin - actorExtent, actorOrigin + actorExtent);

					if(actorBounds.IsInside(nodeBounds))
						isCollidingWithOtherActor = true;

					
					
				}

				if(!isCollidingWithOtherActor) {
					g_graph.nodes.Add(nodeSpawned->GetActorLocation(), nodeSpawned);
				} else {
					//UE_LOG(LogTemp, Warning, TEXT("Collision At: X:%f Y:%f Z:%f"), location.X, location.Y, location.Z);
					nodeSpawned->Destroy();
				}

			}
		}
	}
	

}

void AMyProjectCharacter::IncreaseNumberOfNodes()
{
	spaceDelta += 50.0;
	GEngine->AddOnScreenDebugMessage(-1, 6, FColor::Yellow, FString::Printf(TEXT("Current spaceDelta : %f"), spaceDelta), false);

}


void AMyProjectCharacter::DecreaseNumberOfNodes()
{
	spaceDelta -= 50.0;
	GEngine->AddOnScreenDebugMessage(-1, 6, FColor::Yellow, FString::Printf(TEXT("Current spaceDelta : %f"), spaceDelta), false);

}

void AMyProjectCharacter::Reset()
{

	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, FString::Printf(TEXT("Reseting...")), false);

	for(const auto& ref : g_graph.nodes) {
		ref.Value->Destroy();
	}

	for(auto& kv : g_graph.mapGraph) {
		kv.second.clear();
	}

	g_graph.mapGraph.clear();
	g_graph.nodes.Empty();

	graphVariables.reset();

	startPosition = FVector(0, 0, 0);
	endPosition = FVector(0, 0, 0);

}

void AMyProjectCharacter::MoveCharacterSmoothly(const FVector& _targetPosition, float speed)
{

	if(targetPosition != FVector(0,0,0)) {
		return;
	}

	if(nodePath.Num() == 0) {
		return;
	}

	GetCharacterMovement()->GravityScale = 0.0f;

	targetPosition = nodePath.Pop(0);

	lerpAlpha = 0.0f;
	movementSpeed = speed;

	SetCharacterCollisionEnabled(false);

	// Chame a função UpdateMoveCharacterSmoothly a cada frame
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMyProjectCharacter::UpdateMoveCharacterSmoothly, GetWorld()->GetDeltaSeconds(), true);
}

void AMyProjectCharacter::UpdateMoveCharacterSmoothly()
{

	//FVector nextPosition = FMath::Lerp(GetActorLocation(), targetPosition, lerpAlpha);

	FVector nextPosition = getNextMovePosition(targetPosition);


	//GEngine->AddOnScreenDebugMessage(-1, 0.6, FColor::Yellow, FString::Printf(TEXT("alfa: %f dist: %f newPosition: (%f, %f, %f)"),lerpAlpha, FVector::Distance(nextPosition, targetPosition), nextPosition.X, nextPosition.Y, nextPosition.Z), true);
	if(FVector::Distance(nextPosition, targetPosition) > 10) {

		SetActorLocation(nextPosition);
	} else {

		if(nodePath.Num() != 0) {
			targetPosition = nodePath.Pop();
		} else {
			SetCharacterCollisionEnabled(true);
			GetWorldTimerManager().ClearTimer(timerHandle);
			GetCharacterMovement()->GravityScale = 1.0f;
			targetPosition = FVector(0, 0, 0);
		}
	}
}

FVector AMyProjectCharacter::getNextMovePosition(const FVector& _targetPosition)
{
	float x, y, z;
	FVector currentPosition = GetActorLocation();
	x = (_targetPosition.X > currentPosition.X) ? movementSpeed : -movementSpeed;
	y = (_targetPosition.Y > currentPosition.Y) ? movementSpeed : -movementSpeed;
	z = (_targetPosition.Z > currentPosition.Z) ? movementSpeed : -movementSpeed;


	if(std::abs(_targetPosition.X - currentPosition.X) <= 5) {
		x = 0;
	}

	if(std::abs(_targetPosition.Y - currentPosition.Y) <= 5) {
		y = 0;
	}

	if(std::abs(_targetPosition.Z - currentPosition.Z) <= 5) {
		z = 0;
	}

	return FVector(currentPosition.X + x, currentPosition.Y + y, currentPosition.Z + z);
}

void AMyProjectCharacter::SetCharacterCollisionEnabled(bool bEnabled)
{
	GetCapsuleComponent()->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}


void AMyProjectCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}


void AMyProjectCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AMyProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMyProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMyProjectCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMyProjectCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
