// Fill out your copyright notice in the Description page of Project Settings.


#include "SMonsterData.h"

FPrimaryAssetId USMonsterData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("Monsters", GetFName());
}