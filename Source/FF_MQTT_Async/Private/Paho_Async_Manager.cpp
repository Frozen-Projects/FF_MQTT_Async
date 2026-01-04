// Fill out your copyright notice in the Description page of Project Settings.

#include "Paho_Async_Manager.h"

// Sets default values
APaho_Manager_Async::APaho_Manager_Async()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned.
void APaho_Manager_Async::BeginPlay()
{
	Super::BeginPlay();
}

// Called when the game end or when destroyed.
void APaho_Manager_Async::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	this->MQTT_Async_Destroy();
	Super::EndPlay(EndPlayReason);
}

// Called every frame.
void APaho_Manager_Async::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FPahoClientParams APaho_Manager_Async::GetClientParams()
{
	return this->Client_Params;
}

void APaho_Manager_Async::MQTT_Async_Destroy()
{
	if (!this->Client)
	{
		return;
	}

	if (!MQTTAsync_isConnected(this->Client))
	{
		return;
	}

	try
	{
		MQTTAsync_disconnectOptions Disconnect_Options = MQTTAsync_disconnectOptions_initializer;
		Disconnect_Options.context = this;
		Disconnect_Options.timeout = 10000;

		if (this->Client_Params.Version == EMQTTVERSION::V_5)
		{
			Disconnect_Options.onSuccess5 = OnDisconnect5;
			Disconnect_Options.onFailure5 = OnDisconnectFailure5;
			Disconnect_Options.reasonCode = MQTTREASONCODE_DISCONNECT_WITH_WILL_MESSAGE;
		}

		else
		{
			Disconnect_Options.onSuccess = OnDisconnect;
			Disconnect_Options.onFailure = OnDisconnectFailure;
		}

		MQTTAsync_disconnect(this->Client, &Disconnect_Options);
		MQTTAsync_destroy(&this->Client);
	}

	catch (const std::exception& Exception)
	{
		FJsonObjectWrapper Out_Code;
		Out_Code.JsonObject->SetStringField("PluginName", "FF_MQTT_Async");
		Out_Code.JsonObject->SetStringField("FunctionName", TEXT(__FUNCTION__));
		TArray<TSharedPtr<FJsonValue>> Details;

		const FString ExceptionString = StringCast<TCHAR>(Exception.what()).Get();
		const FString ErrorString = FString::Printf(TEXT("An exception occurred while disconnecting the client: %s"), *ExceptionString);
		
		Details.Add(MakeShared<FJsonValueString>(ErrorString));
		Out_Code.JsonObject->SetArrayField("Details", Details);

		FString JSON_Output;
		Out_Code.JsonObjectToString(JSON_Output);

		UE_LOG(LogTemp, Warning, TEXT("%s"), *JSON_Output);
	}
}

bool APaho_Manager_Async::MQTT_Async_Init(FJsonObjectWrapper& Out_Code, FPahoClientParams In_Params)
{
	Out_Code.JsonObject->SetStringField("PluginName", "FF_MQTT_Async");
	Out_Code.JsonObject->SetStringField("FunctionName", TEXT(__FUNCTION__));
	TArray<TSharedPtr<FJsonValue>> Details;

	if (this->Client)
	{
		Details.Add(MakeShared<FJsonValueString>("Client already initialized !"));
		Out_Code.JsonObject->SetArrayField("Details", Details);
		return false;
	}

	FString ParameterReason;
	if (!In_Params.IsParamsValid(ParameterReason))
	{
		Details.Add(MakeShared<FJsonValueString>(ParameterReason));
		Out_Code.JsonObject->SetArrayField("Details", Details);
		return false;
	}

	FString Protocol = In_Params.GetProtocol();

	MQTTAsync Temp_Client = nullptr;

	MQTTAsync_createOptions Create_Options;

	switch (In_Params.Version)
	{
		case EMQTTVERSION::V3_1:

			Create_Options = MQTTAsync_createOptions_initializer;
			Create_Options.MQTTVersion = MQTTVERSION_3_1;

			if (Protocol == "wss" || Protocol == "ws")
			{
				this->Connection_Options = MQTTAsync_connectOptions_initializer_ws;
			}

			else
			{
				this->Connection_Options = MQTTAsync_connectOptions_initializer;
			}

			this->Connection_Options.cleansession = 1;
			this->Connection_Options.onSuccess = OnConnect;
			this->Connection_Options.onFailure = OnConnectFailure;

			break;

		case EMQTTVERSION::V3_1_1:

			Create_Options = MQTTAsync_createOptions_initializer;
			Create_Options.MQTTVersion = MQTTVERSION_3_1_1;

			if (Protocol == "wss" || Protocol == "ws")
			{
				this->Connection_Options = MQTTAsync_connectOptions_initializer_ws;
			}

			else
			{
				this->Connection_Options = MQTTAsync_connectOptions_initializer;
			}

			this->Connection_Options.cleansession = 1;
			this->Connection_Options.onSuccess = OnConnect;
			this->Connection_Options.onFailure = OnConnectFailure;

			break;

		case EMQTTVERSION::V_5:

			Create_Options = MQTTAsync_createOptions_initializer;
			Create_Options.MQTTVersion = MQTTVERSION_5;

			if (Protocol == "wss" || Protocol == "ws")
			{
				this->Connection_Options = MQTTAsync_connectOptions_initializer5_ws;
			}

			else
			{
				this->Connection_Options = MQTTAsync_connectOptions_initializer5;
			}

			this->Connection_Options.cleanstart = 1;
			this->Connection_Options.onSuccess5 = OnConnect5;
			this->Connection_Options.onFailure5 = OnConnectFailure5;

			break;

		default:

			Create_Options = MQTTAsync_createOptions_initializer;
			Create_Options.MQTTVersion = MQTTVERSION_3_1_1;

			if (Protocol == "wss" || Protocol == "ws")
			{
				this->Connection_Options = MQTTAsync_connectOptions_initializer_ws;
			}

			else
			{
				this->Connection_Options = MQTTAsync_connectOptions_initializer;
			}

			this->Connection_Options.cleansession = 1;
			this->Connection_Options.onSuccess = OnConnect;
			this->Connection_Options.onFailure = OnConnectFailure;

			break;
	}

	int RetVal = MQTTAsync_createWithOptions(&Temp_Client, (const char*)StringCast<UTF8CHAR>(*In_Params.Address).Get(), (const char*)StringCast<UTF8CHAR>(*In_Params.ClientId).Get(), MQTTCLIENT_PERSISTENCE_NONE, NULL, &Create_Options);

	if (RetVal != MQTTASYNC_SUCCESS)
	{
		MQTTAsync_destroy(&Temp_Client);

		Details.Add(MakeShared<FJsonValueString>(FString::Printf(TEXT("MQTTAsync_createWithOptions failed with error code %d"), RetVal)));
		Out_Code.JsonObject->SetArrayField("Details", Details);
		return false;
	}

	this->Connection_Options.context = this;
	this->Connection_Options.keepAliveInterval = In_Params.KeepAliveInterval;
	this->Connection_Options.username = (const char*)StringCast<UTF8CHAR>(*In_Params.UserName).Get();
	this->Connection_Options.password = (const char*)StringCast<UTF8CHAR>(*In_Params.Password).Get();
	this->Connection_Options.MQTTVersion = (int32)In_Params.Version;
	
	if (this->SetSSLParams(Protocol, In_Params.SSL_Options))
	{
		this->Connection_Options.ssl = &this->SSL_Options;
		Details.Add(MakeShared<FJsonValueString>("SSL parameters set."));
	}

	else
	{
		Details.Add(MakeShared<FJsonValueString>("SSL parameters couldn't be set."));
	}	

	RetVal = MQTTAsync_setCallbacks(Temp_Client, this, APaho_Manager_Async::ConnectionLost, APaho_Manager_Async::MessageArrived, APaho_Manager_Async::MessageDelivered);

	if (RetVal != MQTTASYNC_SUCCESS)
	{
		MQTTAsync_destroy(&Temp_Client);

		Details.Add(MakeShared<FJsonValueString>(FString::Printf(TEXT("MQTTAsync_setCallbacks failed with error code %d"), RetVal)));
		Out_Code.JsonObject->SetArrayField("Details", Details);
		return false;
	}

	RetVal = MQTTAsync_connect(Temp_Client, &this->Connection_Options);

	if (RetVal != MQTTASYNC_SUCCESS)
	{
		MQTTAsync_destroy(&Temp_Client);

		Details.Add(MakeShared<FJsonValueString>(FString::Printf(TEXT("MQTTAsync_connect failed with error code %d"), RetVal)));
		Out_Code.JsonObject->SetArrayField("Details", Details);
		return false;
	}

	this->Client = MoveTemp(Temp_Client);
	this->Client_Params = In_Params;

	Details.Add(MakeShared<FJsonValueString>("Client initialized successfully."));
	Out_Code.JsonObject->SetArrayField("Details", Details);
	return true;
}

bool APaho_Manager_Async::MQTT_Async_Publish(FJsonObjectWrapper& Out_Code, FString In_Topic, FString In_Payload, EMQTTQOS In_QoS, int32 In_Retained)
{
	Out_Code.JsonObject->SetStringField("PluginName", "FF_MQTT_Async");
	Out_Code.JsonObject->SetStringField("FunctionName", TEXT(__FUNCTION__));
	TArray<TSharedPtr<FJsonValue>> Details;

	if (!this->Client)
	{
		Details.Add(MakeShared<FJsonValueString>("Client is not valid !"));
		Out_Code.JsonObject->SetArrayField("Details", Details);
		return false;
	}

	if (!MQTTAsync_isConnected(this->Client))
	{
		Details.Add(MakeShared<FJsonValueString>("Client is not connected !"));
		Out_Code.JsonObject->SetArrayField("Details", Details);
		return false;
	}

	MQTTAsync_message PublishedMessage = MQTTAsync_message_initializer;
	PublishedMessage.payload = (void*)(const char*)StringCast<UTF8CHAR>(*In_Payload).Get();
	PublishedMessage.payloadlen = In_Payload.Len();
	PublishedMessage.qos = 0;
	PublishedMessage.retained = 0;

	MQTTAsync_responseOptions ResponseOptions = MQTTAsync_responseOptions_initializer;
	ResponseOptions.context = this;
	
	if (this->Client_Params.Version == EMQTTVERSION::V_5)
	{
		ResponseOptions.onSuccess5 = OnSend5;
		ResponseOptions.onFailure5 = OnSendFailure5;
	}

	else
	{
		ResponseOptions.onSuccess = OnSend;
		ResponseOptions.onFailure = OnSendFailure;
	}
	
	const int RetVal = MQTTAsync_sendMessage(this->Client, (const char*)StringCast<UTF8CHAR>(*In_Topic).Get(), &PublishedMessage, &ResponseOptions);

	const FString ResultString = RetVal == MQTTASYNC_SUCCESS ? "Payload successfully published." : FString::Printf(TEXT("There was a problem while publishing payload with these configurations : %d"), RetVal);
	Details.Add(MakeShared<FJsonValueString>(ResultString));
	Out_Code.JsonObject->SetArrayField("Details", Details);
	
	return RetVal == MQTTASYNC_SUCCESS ? true : false;
}

bool APaho_Manager_Async::MQTT_Async_Subscribe(FJsonObjectWrapper& Out_Code, FString In_Topic, EMQTTQOS In_QoS)
{
	Out_Code.JsonObject->SetStringField("PluginName", "FF_MQTT_Async");
	Out_Code.JsonObject->SetStringField("FunctionName", TEXT(__FUNCTION__));
	TArray<TSharedPtr<FJsonValue>> Details;

	if (!this->Client)
	{
		Details.Add(MakeShared<FJsonValueString>("Client is not valid !"));
		Out_Code.JsonObject->SetArrayField("Details", Details);
		return false;
	}

	if (!MQTTAsync_isConnected(this->Client))
	{
		Details.Add(MakeShared<FJsonValueString>("Client is not connected !"));
		Out_Code.JsonObject->SetArrayField("Details", Details);
		return false;
	}

	MQTTAsync_responseOptions Response_Options = MQTTAsync_responseOptions_initializer;
	Response_Options.context = this;

	if (this->Client_Params.Version == EMQTTVERSION::V_5)
	{
		Response_Options.onSuccess5 = APaho_Manager_Async::OnSubscribe5;
		Response_Options.onFailure5 = APaho_Manager_Async::OnSubscribeFailure5;
	}

	else
	{
		Response_Options.onSuccess = APaho_Manager_Async::OnSubscribe;
		Response_Options.onFailure = APaho_Manager_Async::OnSubscribeFailure;
	}

	const int RetVal = MQTTAsync_subscribe(this->Client, (const char*)StringCast<UTF8CHAR>(*In_Topic).Get(), (int32)In_QoS, &Response_Options);

	const FString ResultString = RetVal == MQTTASYNC_SUCCESS ? "Topic successfully subscribed." : FString::Printf(TEXT("There was a problem while subscribing topic with these configurations. : %d"), RetVal);
	Details.Add(MakeShared<FJsonValueString>(ResultString));
	Out_Code.JsonObject->SetArrayField("Details", Details);

	return RetVal == MQTTASYNC_SUCCESS ? true : false;
}

bool APaho_Manager_Async::MQTT_Async_Unsubscribe(FJsonObjectWrapper& Out_Code, FString In_Topic)
{
	Out_Code.JsonObject->SetStringField("PluginName", "FF_MQTT_Async");
	Out_Code.JsonObject->SetStringField("FunctionName", TEXT(__FUNCTION__));
	TArray<TSharedPtr<FJsonValue>> Details;

	if (!this->Client)
	{
		Details.Add(MakeShared<FJsonValueString>("Client is not valid !"));
		Out_Code.JsonObject->SetArrayField("Details", Details);
		return false;
	}

	if (!MQTTAsync_isConnected(this->Client))
	{
		Details.Add(MakeShared<FJsonValueString>("Client is not connected !"));
		Out_Code.JsonObject->SetArrayField("Details", Details);
		return false;
	}

	MQTTAsync_responseOptions Response_Options = MQTTAsync_responseOptions_initializer;
	Response_Options.context = this;

	if (this->Client_Params.Version == EMQTTVERSION::V_5)
	{
		Response_Options.onSuccess5 = OnUnSubscribe5;
		Response_Options.onFailure5 = OnUnSubscribeFailure5;
	}

	else
	{
		Response_Options.onSuccess = OnUnSubscribe;
		Response_Options.onFailure = OnUnSubscribeFailure;
	}

	const int RetVal = MQTTAsync_unsubscribe(this->Client, (const char*)StringCast<UTF8CHAR>(*In_Topic).Get(), &Response_Options);

	const FString ResultString = RetVal == MQTTASYNC_SUCCESS ? "Topic successfully unsubscribed." : FString::Printf(TEXT("There was a problem while unsubscribing topic with these configurations. : %d"), RetVal);
	Details.Add(MakeShared<FJsonValueString>(ResultString));
	Out_Code.JsonObject->SetArrayField("Details", Details);

	return RetVal == MQTTASYNC_SUCCESS ? true : false;
}