// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "Generic_Includes.h"
#include "Paho_Async_Includes.h"

#include "Paho_Async_Manager.generated.h"

UCLASS()
class FF_MQTT_ASYNC_API APaho_Manager_Async : public AActor
{
	GENERATED_BODY()
	
private:	

	MQTTAsync Client = nullptr;
	MQTTAsync_connectOptions Connection_Options;
	MQTTAsync_SSLOptions SSL_Options;
	FPahoClientParams Client_Params;

#pragma region Internals

	virtual bool SetSSLParams(FString In_Protocol, FPahoSslOptions In_Options);

#pragma endregion Internals

#pragma region Main_Callbacks

	static void MessageDelivered(void* CallbackContext, MQTTAsync_token DeliveredToken);
	static int MessageArrived(void* CallbackContext, char* TopicName, int TopicLenght, MQTTAsync_message* Message);
	static void ConnectionLost(void* CallbackContext, char* Cause);

#pragma endregion Main_Callbacks

#pragma region V3_Callbacks

	static void OnConnect(void* CallbackContext, MQTTAsync_successData* Response);
	static void OnConnectFailure(void* CallbackContext, MQTTAsync_failureData* Response);
	
	static void OnDisconnect(void* CallbackContext, MQTTAsync_successData* Response);
	static void OnDisconnectFailure(void* CallbackContext, MQTTAsync_failureData* Response);
	
	static void OnSend(void* CallbackContext, MQTTAsync_successData* Response);
	static void OnSendFailure(void* CallbackContext, MQTTAsync_failureData* Response);
	
	static void OnUnSubscribe(void* CallbackContext, MQTTAsync_successData* Response);
	static void OnUnSubscribeFailure(void* CallbackContext, MQTTAsync_failureData* Response);

	static void OnSubscribe(void* CallbackContext, MQTTAsync_successData* Response);
	static void OnSubscribeFailure(void* CallbackContext, MQTTAsync_failureData* Response);

#pragma endregion V3_Callbacks

#pragma region V5_Callbacks

	static void OnConnect5(void* CallbackContext, MQTTAsync_successData5* Response);
	static void OnConnectFailure5(void* CallbackContext, MQTTAsync_failureData5* Response);
	
	static void OnDisconnect5(void* CallbackContext, MQTTAsync_successData5* Response);
	static void OnDisconnectFailure5(void* CallbackContext, MQTTAsync_failureData5* Response);
	
	static void OnSend5(void* CallbackContext, MQTTAsync_successData5* Response);
	static void OnSendFailure5(void* CallbackContext, MQTTAsync_failureData5* Response);
	
	static void OnUnSubscribe5(void* CallbackContext, MQTTAsync_successData5* Response);
	static void OnUnSubscribeFailure5(void* CallbackContext, MQTTAsync_failureData5* Response);

	static void OnSubscribe5(void* CallbackContext, MQTTAsync_successData5* Response);
	static void OnSubscribeFailure5(void* CallbackContext, MQTTAsync_failureData5* Response);
	
#pragma endregion V5_Callbacks

protected:

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game end or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	// Sets default values for this actor's properties.
	APaho_Manager_Async();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Int Delegate_Message_Delivered;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Json Delegate_Message_Arrived;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_String Delegate_Connection_Lost;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Json Delegate_OnConnect;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Json Delegate_OnConnect_Failure;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Json Delegate_OnDisconnect;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Json Delegate_OnDisconnect_Failure;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Json Delegate_OnSend;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Json Delegate_OnSend_Failure;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Json Delegate_OnUnSubscribe;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Json Delegate_OnUnSubscribe_Failure;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Json Delegate_OnSubscribe;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Json Delegate_OnSubscribe_Failure;

	UFUNCTION(BlueprintPure, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Async - Get Client Parameters"))
	virtual FPahoClientParams GetClientParams();

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Async - Destroy", ToolTip = "", KeyWords = "mqtt, async, paho, client, destroy, close, disconnect"))
	virtual void MQTT_Async_Destroy();

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Async - Init", ToolTip = "Don't attach publishers or subscribers immediately after this. Use some delay or better use it after \"Delegate OnConnect\"", KeyWords = "mqtt, async, paho, client, init, initialize, start, connect"))
	virtual void MQTT_Async_Init(FJsonObjectWrapper& Out_Code, bool& bIsSucessful, FPahoClientParams In_Params);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Async - Publish", ToolTip = "Don't use it immediately after \"MQTT Async Init\" give some delay or better use it after \"Delegate OnConnect\"", KeyWords = "mqtt, async, paho, client, publish, publisher"))
	virtual void MQTT_Async_Publish(FJsonObjectWrapper& Out_Code, bool& bIsSucessful, FString In_Topic, FString In_Payload, EMQTTQOS In_QoS = EMQTTQOS::QoS_0, bool bIsRetained = false);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Async - Subscribe", ToolTip = "Don't use it immediately after \"MQTT Async Init\" give some delay or better use it after \"Delegate OnConnect\"", KeyWords = "mqtt, async, paho, client, subscribe, subscriber"))
	virtual void MQTT_Async_Subscribe(FJsonObjectWrapper& Out_Code, bool& bIsSucessful, FString In_Topic, EMQTTQOS In_QoS = EMQTTQOS::QoS_0);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Async - Unsubscribe", ToolTip = "Don't use it immediately after \"MQTT Async Init\" give some delay or better use it after \"Delegate OnConnect\"", KeyWords = "mqtt, async, paho, client, unsubscribe, subscriber"))
	virtual void MQTT_Async_Unsubscribe(FJsonObjectWrapper& Out_Code, bool& bIsSucessful, FString In_Topic);

};
