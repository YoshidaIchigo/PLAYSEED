#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayseedGlideBoard.generated.h"

class UStaticMeshComponent;

// 敵から落ちるオリジナルの滑走葉です。Worldが所有し、再開時は生成元が片付けます。
UCLASS()
class PLAYSEED_API APlayseedGlideBoard : public AActor
{
    GENERATED_BODY()
public:
    APlayseedGlideBoard();
    virtual void BeginPlay() override;
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Leaf;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Ridge;
};
