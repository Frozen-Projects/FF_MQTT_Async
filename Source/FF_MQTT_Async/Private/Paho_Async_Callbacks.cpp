#include "Paho_Async_Manager.h"

#pragma region Internals

bool APaho_Manager_Async::SetSSLParams(FString In_Protocol, FPahoSslOptions In_Options)
{
	if (In_Protocol == "wss" || In_Protocol == "mqtts" || In_Protocol == "ssl" || In_Protocol == "WSS" || In_Protocol == "MQTTS" || In_Protocol == "SSL")
	{
		this->SSL_Options = MQTTAsync_SSLOptions_initializer;
		this->SSL_Options.enableServerCertAuth = 0;
		this->SSL_Options.verify = 1;

		if (!In_Options.CAPath.IsEmpty() && FPaths::FileExists(In_Options.CAPath))
		{
			this->SSL_Options.CApath = (const char*)StringCast<UTF8CHAR>(*In_Options.CAPath).Get();
		}

		if (!In_Options.Path_KeyStore.IsEmpty() && FPaths::FileExists(In_Options.Path_KeyStore))
		{
			this->SSL_Options.keyStore = (const char*)StringCast<UTF8CHAR>(*In_Options.Path_KeyStore).Get();
		}

		if (!In_Options.Path_TrustStore.IsEmpty() && FPaths::FileExists(In_Options.Path_TrustStore))
		{
			this->SSL_Options.trustStore = (const char*)StringCast<UTF8CHAR>(*In_Options.Path_TrustStore).Get();
		}

		if (!In_Options.Path_PrivateKey.IsEmpty() && FPaths::FileExists(In_Options.Path_PrivateKey))
		{
			this->SSL_Options.privateKey = (const char*)StringCast<UTF8CHAR>(*In_Options.Path_PrivateKey).Get();
		}

		if (!In_Options.PrivateKeyPass.IsEmpty())
		{
			this->SSL_Options.privateKeyPassword = (const char*)StringCast<UTF8CHAR>(*In_Options.PrivateKeyPass).Get();
		}

		if (!In_Options.CipherSuites.IsEmpty())
		{
			this->SSL_Options.enabledCipherSuites = (const char*)StringCast<UTF8CHAR>(*In_Options.CipherSuites).Get();
		}

		return true;
	}

	else
	{
		return false;
	}
}

#pragma endregion Internals

#pragma region Main_Callbacks

void APaho_Manager_Async::MessageDelivered(void* CallbackContext, MQTTAsync_token DeliveredToken)
{
	AsyncTask(ENamedThreads::GameThread, [CallbackContext, DeliveredToken]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!Owner)
		{
			return;
		}

		Owner->Delegate_Message_Delivered.Broadcast(DeliveredToken);
	});
}

int APaho_Manager_Async::MessageArrived(void* CallbackContext, char* TopicName, int TopicLenght, MQTTAsync_message* Message)
{
	const FString TopicNameStr = StringCast<UTF8CHAR>(TopicName).Get();
	const FString PayloadStr = StringCast<UTF8CHAR>((const char*)Message->payload).Get();

	FJsonObjectWrapper Arrived;
	Arrived.JsonObject->SetStringField("TopicName", TopicNameStr);
	Arrived.JsonObject->SetNumberField("TopicLength", TopicLenght);
	Arrived.JsonObject->SetStringField("Message", PayloadStr);

	MQTTAsync_freeMessage(&Message);
	MQTTAsync_free(TopicName);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Arrived]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!Owner)
		{
			return;
		}

		Owner->Delegate_Message_Arrived.Broadcast(Arrived);
	});

	return 1;
}

void APaho_Manager_Async::ConnectionLost(void* CallbackContext, char* Cause)
{
	const FString CauseStr = StringCast<UTF8CHAR>(Cause).Get();

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, CauseStr]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!Owner)
		{
			return;
		}

		Owner->Delegate_Connection_Lost.Broadcast(CauseStr);
	});
}

#pragma endregion Main_Callbacks

#pragma region V3_Callbacks

void APaho_Manager_Async::OnConnect(void* CallbackContext, MQTTAsync_successData* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("MQTT_Version", Response->alt.connect.MQTTVersion);
	Out_Result.JsonObject->SetNumberField("Session_Present", Response->alt.connect.sessionPresent);
	Out_Result.JsonObject->SetStringField("Server_Uri", StringCast<UTF8CHAR>(Response->alt.connect.serverURI).Get());

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);
		
		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnConnect.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnConnectFailure(void* CallbackContext, MQTTAsync_failureData* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("Code", Response->code);
	Out_Result.JsonObject->SetStringField("Message", Response->message);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);
		
		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnConnect_Failure.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnDisconnect(void* CallbackContext, MQTTAsync_successData* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("MQTT_Version", Response->alt.connect.MQTTVersion);
	Out_Result.JsonObject->SetNumberField("Session_Present", Response->alt.connect.sessionPresent);
	Out_Result.JsonObject->SetStringField("Server_Uri", StringCast<UTF8CHAR>(Response->alt.connect.serverURI).Get());

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);
		
		if (!IsValid(Owner))
		{
			return;
		}
		
		Owner->Delegate_OnDisconnect.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnDisconnectFailure(void* CallbackContext, MQTTAsync_failureData* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("Code", Response->code);
	Out_Result.JsonObject->SetStringField("Message", Response->message);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);
		
		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnDisconnect_Failure.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnSend(void* CallbackContext, MQTTAsync_successData* Response)
{
	FJsonObjectWrapper Out_Result;

	if (Response->alt.pub.message.payload && Response->alt.pub.message.payloadlen > 0)
	{
		const FString PayloadStr = StringCast<UTF8CHAR>((const char*)Response->alt.pub.message.payload).Get();
		Out_Result.JsonObject->SetStringField("Payload", PayloadStr);
	}

	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetStringField("Topic", Response->alt.pub.destinationName);
	Out_Result.JsonObject->SetNumberField("Dup_Flag", Response->alt.pub.message.dup);
	Out_Result.JsonObject->SetNumberField("Message_Id", Response->alt.pub.message.msgid);
	Out_Result.JsonObject->SetNumberField("Payload_Length", Response->alt.pub.message.payloadlen);
	Out_Result.JsonObject->SetNumberField("Published_QoS", Response->alt.pub.message.qos);
	Out_Result.JsonObject->SetNumberField("Published_Retained", Response->alt.pub.message.retained);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);
		
		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnSend.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnSendFailure(void* CallbackContext, MQTTAsync_failureData* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("Code", Response->code);
	Out_Result.JsonObject->SetStringField("Message", Response->message);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);
		
		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnSend_Failure.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnUnSubscribe(void* CallbackContext, MQTTAsync_successData* Response)
{
	FJsonObjectWrapper Out_Result;

	if (Response->alt.pub.message.payload && Response->alt.pub.message.payloadlen > 0)
	{
		const FString PayloadStr = StringCast<UTF8CHAR>((const char*)Response->alt.pub.message.payload).Get();
		Out_Result.JsonObject->SetStringField("Payload", PayloadStr);
	}

	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetStringField("Topic", Response->alt.pub.destinationName);
	Out_Result.JsonObject->SetNumberField("Dup_Flag", Response->alt.pub.message.dup);
	Out_Result.JsonObject->SetNumberField("Message_Id", Response->alt.pub.message.msgid);
	Out_Result.JsonObject->SetNumberField("Payload_Length", Response->alt.pub.message.payloadlen);
	Out_Result.JsonObject->SetNumberField("Published_QoS", Response->alt.pub.message.qos);
	Out_Result.JsonObject->SetNumberField("Published_Retained", Response->alt.pub.message.retained);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnUnSubscribe.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnUnSubscribeFailure(void* CallbackContext, MQTTAsync_failureData* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("Code", Response->code);
	Out_Result.JsonObject->SetStringField("Message", Response->message);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnUnSubscribe_Failure.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnSubscribe(void* CallbackContext, MQTTAsync_successData* Response)
{
	FJsonObjectWrapper Out_Result;

	if (Response->alt.pub.message.payload && Response->alt.pub.message.payloadlen > 0)
	{
		const FString PayloadStr = StringCast<UTF8CHAR>((const char*)Response->alt.pub.message.payload).Get();
		Out_Result.JsonObject->SetStringField("Payload", PayloadStr);
	}

	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetStringField("Topic", Response->alt.pub.destinationName);
	Out_Result.JsonObject->SetNumberField("Dup_Flag", Response->alt.pub.message.dup);
	Out_Result.JsonObject->SetNumberField("Message_Id", Response->alt.pub.message.msgid);
	Out_Result.JsonObject->SetNumberField("Payload_Length", Response->alt.pub.message.payloadlen);
	Out_Result.JsonObject->SetNumberField("Published_QoS", Response->alt.pub.message.qos);
	Out_Result.JsonObject->SetNumberField("Published_Retained", Response->alt.pub.message.retained);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnSubscribe.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnSubscribeFailure(void* CallbackContext, MQTTAsync_failureData* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("Code", Response->code);
	Out_Result.JsonObject->SetStringField("Message", Response->message);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnSubscribe_Failure.Broadcast(Out_Result);
	});
}

#pragma endregion V3_Callbacks

#pragma region V5_Callbacks

void APaho_Manager_Async::OnConnect5(void* CallbackContext, MQTTAsync_successData5* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("MQTT_Version", Response->alt.connect.MQTTVersion);
	Out_Result.JsonObject->SetNumberField("Session_Present", Response->alt.connect.sessionPresent);
	Out_Result.JsonObject->SetStringField("Server_Uri", StringCast<UTF8CHAR>(Response->alt.connect.serverURI).Get());

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnConnect.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnConnectFailure5(void* CallbackContext, MQTTAsync_failureData5* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("Code", Response->code);
	Out_Result.JsonObject->SetStringField("Message", Response->message);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnConnect_Failure.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnDisconnect5(void* CallbackContext, MQTTAsync_successData5* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("MQTT_Version", Response->alt.connect.MQTTVersion);
	Out_Result.JsonObject->SetNumberField("Session_Present", Response->alt.connect.sessionPresent);
	Out_Result.JsonObject->SetStringField("Server_Uri", StringCast<UTF8CHAR>(Response->alt.connect.serverURI).Get());

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnDisconnect.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnDisconnectFailure5(void* CallbackContext, MQTTAsync_failureData5* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("Code", Response->code);
	Out_Result.JsonObject->SetStringField("Message", Response->message);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnDisconnect_Failure.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnSend5(void* CallbackContext, MQTTAsync_successData5* Response)
{
	FJsonObjectWrapper Out_Result;

	if (Response->alt.pub.message.payload && Response->alt.pub.message.payloadlen > 0)
	{
		const FString PayloadStr = StringCast<UTF8CHAR>((const char*)Response->alt.pub.message.payload).Get();
		Out_Result.JsonObject->SetStringField("Payload", PayloadStr);
	}

	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetStringField("Topic", Response->alt.pub.destinationName);
	Out_Result.JsonObject->SetNumberField("Dup_Flag", Response->alt.pub.message.dup);
	Out_Result.JsonObject->SetNumberField("Message_Id", Response->alt.pub.message.msgid);
	Out_Result.JsonObject->SetNumberField("Payload_Length", Response->alt.pub.message.payloadlen);
	Out_Result.JsonObject->SetNumberField("Published_QoS", Response->alt.pub.message.qos);
	Out_Result.JsonObject->SetNumberField("Published_Retained", Response->alt.pub.message.retained);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnSend.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnSendFailure5(void* CallbackContext, MQTTAsync_failureData5* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("Code", Response->code);
	Out_Result.JsonObject->SetStringField("Message", Response->message);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnSend_Failure.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnUnSubscribe5(void* CallbackContext, MQTTAsync_successData5* Response)
{
	FJsonObjectWrapper Out_Result;

	if (Response->alt.pub.message.payload && Response->alt.pub.message.payloadlen > 0)
	{
		const FString PayloadStr = StringCast<UTF8CHAR>((const char*)Response->alt.pub.message.payload).Get();
		Out_Result.JsonObject->SetStringField("Payload", PayloadStr);
	}

	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetStringField("Topic", Response->alt.pub.destinationName);
	Out_Result.JsonObject->SetNumberField("Dup_Flag", Response->alt.pub.message.dup);
	Out_Result.JsonObject->SetNumberField("Message_Id", Response->alt.pub.message.msgid);
	Out_Result.JsonObject->SetNumberField("Payload_Length", Response->alt.pub.message.payloadlen);
	Out_Result.JsonObject->SetNumberField("Published_QoS", Response->alt.pub.message.qos);
	Out_Result.JsonObject->SetNumberField("Published_Retained", Response->alt.pub.message.retained);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnUnSubscribe.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnUnSubscribeFailure5(void* CallbackContext, MQTTAsync_failureData5* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("Code", Response->code);
	Out_Result.JsonObject->SetStringField("Message", Response->message);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnUnSubscribe_Failure.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnSubscribe5(void* CallbackContext, MQTTAsync_successData5* Response)
{
	FJsonObjectWrapper Out_Result;

	if (Response->alt.pub.message.payload && Response->alt.pub.message.payloadlen > 0)
	{
		const FString PayloadStr = StringCast<UTF8CHAR>((const char*)Response->alt.pub.message.payload).Get();
		Out_Result.JsonObject->SetStringField("Payload", PayloadStr);
	}

	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetStringField("Topic", Response->alt.pub.destinationName);
	Out_Result.JsonObject->SetNumberField("Dup_Flag", Response->alt.pub.message.dup);
	Out_Result.JsonObject->SetNumberField("Message_Id", Response->alt.pub.message.msgid);
	Out_Result.JsonObject->SetNumberField("Payload_Length", Response->alt.pub.message.payloadlen);
	Out_Result.JsonObject->SetNumberField("Published_QoS", Response->alt.pub.message.qos);
	Out_Result.JsonObject->SetNumberField("Published_Retained", Response->alt.pub.message.retained);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnSubscribe.Broadcast(Out_Result);
	});
}

void APaho_Manager_Async::OnSubscribeFailure5(void* CallbackContext, MQTTAsync_failureData5* Response)
{
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetNumberField("Token", Response->token);
	Out_Result.JsonObject->SetNumberField("Code", Response->code);
	Out_Result.JsonObject->SetStringField("Message", Response->message);

	AsyncTask(ENamedThreads::GameThread, [CallbackContext, Out_Result]()
	{
		APaho_Manager_Async* Owner = Cast<APaho_Manager_Async>((APaho_Manager_Async*)CallbackContext);

		if (!IsValid(Owner))
		{
			return;
		}

		Owner->Delegate_OnSubscribe_Failure.Broadcast(Out_Result);
	});
}

#pragma endregion V5_Callbacks