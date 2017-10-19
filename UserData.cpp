#include <time.h>
#include <Arduino.h>
#include "UserData.h"

UserData::UserData()
{
    Next = NULL;
    for (int i=0; i<11; i++)
    {
        DIF[i] = VIF[i] = 0;
    }
}

UserData::~UserData()
{
    delete(this->Next);
}

float UserData::DataAsFloat(float pFactor)
{
    long vValue = 0;
    switch (DIF[0] & 0b1111)
    {
        case 0b0000:
			Serial1.println("- No data -: ");
            break;
        case 0b0001:
        case 0b0010:
        case 0b0011:
        case 0b0100:
            vValue = DataAsInteger();
            break;
        case 0b1001:
        case 0b1010:
        case 0b1011:
        case 0b1100:
        case 0b1110:
            vValue = DataAsBCDInteger();
            break;
        default:
			Serial1.println("Unknown data type: ");
            break;
    }

    return vValue * pFactor;
}


unsigned long UserData::DataAsInteger()
{
    unsigned long vValue = 0;
    for (int i=DataLength-1; i>=0; i--)
    {
        vValue *= 0x100;
        vValue += Data[i];
    }
    return vValue;
}


unsigned long UserData::DataAsBCDInteger()
{
    unsigned long vValue = 0;
    // for (int i=0; i<DataLength; i++)
    for (int i=DataLength-1; i>=0; i--)
    {
        vValue *= 10;
        vValue += (Data[i] & 0xF0) >> 4;
        vValue *= 10;
        vValue += (Data[i] & 0x0F);
    }
    return vValue;
}

time_t UserData::toUnixTime(int pYear, int pMonth, int pDay, int pHour, int pMinute, int pSeconds)
{
	byte vDaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	long vSecondsPerMinute = 60;
	long vSecondsPerHour = vSecondsPerMinute * 60;
	long vSecondsPerDay = vSecondsPerHour * 24;

	long vTime = (pYear - 1970) * vSecondsPerDay * 365L;

	for (int vYear = 1970; vYear<pYear; vYear++)
		if ((vYear % 4 == 0) && ((vYear % 100 != 0) || (vYear % 400 == 0)))
			vTime += vSecondsPerDay;

	if (pMonth > 2 && (pYear % 4 == 0) && ((pYear % 100 != 0) || (pYear % 400 == 0)))
		vTime += vSecondsPerDay;

	for (int vMonth = 1; vMonth<pMonth; vMonth++)
		vTime += vDaysInMonth[vMonth - 1] * vSecondsPerDay;

	vTime += (pDay - 1) * vSecondsPerDay;
	vTime += pHour * vSecondsPerHour;
	vTime += pMinute * vSecondsPerMinute;
	vTime += pSeconds;

	return (time_t)vTime;
}

time_t UserData::DataAsDate_F()
{
    if (DataLength != 4)
    {
		Serial1.println("Incompatible Data Length for Date Type F: ");
		Serial1.println(DataLength);
		return 0;
    }
    else
    {
		int vMinutes = (Data[0] & 0b00111111);
		int vHours = (Data[1] & 0b00011111);
		int vDay = (Data[2] & 0b00011111);
		int vMonth = (Data[3] & 0b00001111);
		int vCentury = (Data[1] & 0b01100000) >> 5;
		int vYear = 1900 + (((Data[2] & 0b11100000) >> 5) | ((Data[3] & 0b11110000) >> 1)) + (100 * vCentury);
		if (vYear < 1950)
			vYear += 100;
		return toUnixTime(vYear, vMonth, vDay, vHours, vMinutes, 0);
    }
    
}

time_t UserData::DataAsDate_G()
{
    tm tmp;
    tmp.tm_mday = 0; // Assume invalid date

    if (DataLength != 2)
    {
		Serial1.println("Incompatible Data Length for Date Type G: ");
		Serial1.println(DataLength);
    }
    else if (Data[0] == 0xff && Data[1] == 0xff)
    {
		// This happens some times. Invalid date, but what to do?
    }
    else
    {
        tmp.tm_min = 0;
        tmp.tm_hour = 0;
        tmp.tm_mday = (Data[0] & 0b00011111);
        tmp.tm_mon = (Data[1] & 0b00001111);
        tmp.tm_year = ((Data[0] & 0b11100000) >> 5) | ((Data[1] & 0b11110000) >> 1);

    }
	return mktime(&tmp);
}


time_t UserData::DataAsDate_I()
{
    tm tmp;
    if (DataLength != 6)
    {
		Serial1.println("Incompatible Data Length for Date Type I: ");
		Serial1.println(DataLength);
    }
    else
    {
        tmp.tm_mday = (Data[0] & 0b00011111);
        tmp.tm_mon = (Data[1] & 0b00001111) >> 1;
        tmp.tm_year = ((Data[0] & 0b11100000) >> 1) & (Data[1] & 0b11110000);
    }
	return mktime(&tmp);
}

time_t UserData::DataAsDate_J()
{
    tm tmp;
    if (DataLength != 3)
    {
		Serial1.println("Incompatible Data Length for Date Type J: ");
		Serial1.println(DataLength);
    }
    else
    {
        tmp.tm_mday = (Data[0] & 0b00011111);
        tmp.tm_mon = (Data[1] & 0b00001111) >> 1;
        tmp.tm_year = ((Data[0] & 0b11100000) >> 1) & (Data[1] & 0b11110000);
    }
	return mktime(&tmp);
}

void UserData::print(tm tmp)
{
    if (tmp.tm_mday < 10)
        Serial1.println(0);
	Serial1.println(tmp.tm_mday);
	Serial1.println(".");

    if (tmp.tm_mon < 10)
        Serial1.println(0);
	Serial1.println(tmp.tm_mon);
	Serial1.println(".");

    if (tmp.tm_year < 10)
		Serial1.println(0);
	Serial1.println(tmp.tm_year);

    if (tmp.tm_min != 0 || tmp.tm_hour != 0)
    {
		Serial1.println(" ");
        if (tmp.tm_hour < 10)
			Serial1.println(0);
		Serial1.println(tmp.tm_hour);
		Serial1.println(":");

        if (tmp.tm_min < 10)
			Serial1.println(0);
		Serial1.println(tmp.tm_min);
    }
}

void UserData::debug()
{
	Serial1.println("  DIF: ");
    for (int i=0; i<11; i++)
    {
        if (DIF[i])
        {
			Serial1.println(DIF[i], HEX);
			Serial1.println(" ");
        }
    }
	Serial1.println(" / VIF: ");
    for (int i=0; i<11; i++)
    {
        if (VIF[i])
        {
			Serial1.println(VIF[i], HEX);
			Serial1.println(" ");
        }
    }

    if (this->DataLength > 0)
    {
		Serial1.println(" / Data ( ");
		Serial1.println(this->DataLength);
		Serial1.println("bytes): ");
        for (int i=0; i<this->DataLength; i++)
        {
			Serial1.println(" ");
			Serial1.println(this->Data[i], HEX);
        }
    }

    switch (DIF[0] & 0b00110000)
    {
        case 0b00000000:
			Serial1.println("  Instantaneous value");
            break;
        case 0b00010000:
			Serial1.println("  Maximum value");
            break;
        case 0b00100000:
			Serial1.println("  Minimum value");
            break;
        case 0b00110000:
			Serial1.println("  Value during error state");
            break;
    }
}


void UserData::parse()
{
    this->Type = UserDataType_Unknown;
    this->Unit = UserDataUnit_Unknown;
    this->Value = 0;
    float vFactor = 0;

    this->Storage = (DIF[0] & 0b01000000) >> 6;
    int i=0;
    while (DIF[i] & 0b10000000)
    {
        this->Storage |= ((DIF[i+1] & 0b00001111) << (1 + 4 * i));
        i++;
    }

    if ((VIF[0] & 0b01111000) == 0b0000000)
    {
        // Energy
        this->Type = UserDataType_Energy1;
        vFactor = pow(10, (VIF[0] & 0b111) - 3);
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_WattHours;
    }
    else if ((VIF[0] & 0b01111000) == 0b0001000)
    {
        // Energy
        this->Type = UserDataType_Energy2;
        vFactor = pow(10, (VIF[0] & 0b111));
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_Joules;
    }
    else if ((VIF[0] & 0b01111000) == 0b0010000)
    {
        // Volume
        this->Type = UserDataType_Volume;
        vFactor = pow(10, (VIF[0] & 0b111) - 6);
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_CubicMeters;
    }
    else if ((VIF[0] & 0b01111000) == 0b0011000)
    {
        // Mass
        this->Type = UserDataType_Mass;
        vFactor = pow(10, (VIF[0] & 0b111) - 3);
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_KiloGrams;
    }
    else if ((VIF[0] & 0b01111100) == 0b0100100)
    {
        // On Time
        this->Type = UserDataType_OnTime;
        this->Value = DataAsFloat(1);

        switch (VIF[0] & 0b11)
        {
            case 0b00:
                this->Unit = UserDataUnit_Seconds; break;
            case 0b01:
                this->Unit = UserDataUnit_Minutes; break;
            case 0b10:
                this->Unit = UserDataUnit_Hours; break;
            case 0b11:
                this->Unit = UserDataUnit_Days; break;
        }
    }
    else if ((VIF[0] & 0b01111000) == 0b0101000)
    {
        // Power
        this->Type = UserDataType_Power1;
        vFactor = pow(10, (VIF[0] & 0b111) - 3);
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_Watts;
    }
    else if ((VIF[0] & 0b01111000) == 0b0110000)
    {
        // Power
        this->Type = UserDataType_Power2;
        vFactor = pow(10, (VIF[0] & 0b111));
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_JoulesPerHour;
    }
    else if ((VIF[0] & 0b01111000) == 0b0111000)
    {
        // Volume Flow
        this->Type = UserDataType_VolumeFlow;
        vFactor = pow(10, (VIF[0] & 0b111) - 6);
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_CubicMetersPerHour;
    }
    else if ((VIF[0] & 0b01111000) == 0b1000000)
    {
        // Volume Flow ext.
        this->Type = UserDataType_VolumeFlowExt1;
        vFactor = pow(10, (VIF[0] & 0b111) - 7);
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_CubicMetersPerMinute;
    }
    else if ((VIF[0] & 0b01111000) == 0b1001000)
    {
        // Volume Flow ext.
        this->Type = UserDataType_VolumeFlowExt2;
        vFactor = pow(10, (VIF[0] & 0b111) - 9);
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_CubicMetersPerSecond;
    }
    else if ((VIF[0] & 0b01111000) == 0b1010000)
    {
        // Mass flow
        this->Type = UserDataType_MassFlow;
        vFactor = pow(10, (VIF[0] & 0b111) - 3);
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_KiloGramsPerHour;
    }
    else if ((VIF[0] & 0b01111100) == 0b1011000)
    {
        // Flow Temperature
        this->Type = UserDataType_FlowTemperature;
        vFactor = pow(10, (VIF[0] & 0b11) - 3);
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_Celcius;
    }
    else if ((VIF[0] & 0b01111100) == 0b1011100)
    {
        // Return Temperature
        this->Type = UserDataType_ReturnTemperature;
        vFactor = pow(10, (VIF[0] & 0b11) - 3);
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_Celcius;
    }
    else if ((VIF[0] & 0b01111100) == 0b1100000)
    {
        // Temperature Difference
        this->Type = UserDataType_TemperatureDifference;
        vFactor = pow(10, (VIF[0] & 0b11) - 3);
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_Kelvin;
    }
    else if ((VIF[0] & 0b01111100) == 0b1100100)
    {
        // External Temperature
        this->Type = UserDataType_ExternalTemperature;
        vFactor = pow(10, (VIF[0] & 0b11) - 3);
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_Celcius;
    }
    else if ((VIF[0] & 0b01111100) == 0b1101000)
    {
        // Pressure
        this->Type = UserDataType_Pressure;
        vFactor = pow(10, (VIF[0] & 0b11) - 3);
        this->Value = DataAsFloat(vFactor);
        this->Unit = UserDataUnit_Bar;
    }
    else if ((VIF[0] & 0b01111111) == 0b1101100)
    {
        // Date
        this->Type = UserDataType_Date;
        this->Value = DataAsDate_G();
        this->Unit = UserDataUnit_Date;
        // data == 0010 => Date, type G
    }
    else if ((VIF[0] & 0b01111111) == 0b01101101)
    {
        if ((this->DIF[0] & 0b1111) == 0b0100)
        {
             // Date and Time
            this->Type = UserDataType_DateAndTime;
            this->Value = DataAsDate_F();
            this->Unit = UserDataUnit_Date;
            // data == 0100 => Date and time, type F
        }
        else if ((this->DIF[0] & 0b1111) == 0b0011)
        {
            // Extended Time Point
            this->Type = UserDataType_ExtendedTimePoint;
            this->Value = DataAsDate_J();
            this->Unit = UserDataUnit_Date;
            // data == 0011 => Date and time, type J
        }
        else if ((this->DIF[0] & 0b1111) == 0b0110)
        {
            // Extended Date and Time Point
            this->Type = UserDataType_ExtendedDateAndTime;
            this->Value = DataAsDate_I();
            this->Unit = UserDataUnit_Date;
            // data == 0110 => Date and time, type I
		}
    }
    else if ((VIF[0] & 0b01111111) == 0b1101110)
    {
        // Units for HCA
        this->Type = UserDataType_UnitsForHCA;
    }
    else if ((VIF[0] & 0b01111111) == 0b1101111)
    {
        // Reserved for future VIF-extensions
        this->Type = UserDataType_Reserved;
    }
    else if ((VIF[0] & 0b01111100) == 0b1110000)
    {
        // Averaging duration
        this->Type = UserDataType_AveragingDuration;
        this->Value = DataAsFloat(1);

        switch (VIF[0] & 0b11)
        {
            case 0b00:
                this->Unit = UserDataUnit_Seconds; break;
            case 0b01:
                this->Unit = UserDataUnit_Minutes; break;
            case 0b10:
                this->Unit = UserDataUnit_Hours; break;
            case 0b11:
                this->Unit = UserDataUnit_Days; break;
        }
    }
    else if ((VIF[0] & 0b01111100) == 0b1110100)
    {
        // Actually duration
        this->Type = UserDataType_ActualityDuration;
        this->Value = DataAsFloat(1);

        switch (VIF[0] & 0b11)
        {
            case 0b00:
                this->Unit = UserDataUnit_Seconds; break;
            case 0b01:
                this->Unit = UserDataUnit_Minutes; break;
            case 0b10:
                this->Unit = UserDataUnit_Hours; break;
            case 0b11:
                this->Unit = UserDataUnit_Days; break;
        }
    }
    else if ((VIF[0] & 0b01111111) == 0b1111000)
    {
        // Fabrication No
        this->Type = UserDataType_FabricationNo;
    }
    else if ((VIF[0] & 0b01111111) == 0b1111001)
    {
        // (Enhanced) Identification
        this->Type = UserDataType_EnhancedIdentification;
        this->Unit = UserDataUnit_BCD;
        this->Value = DataAsBCDInteger();
    }
    else if ((VIF[0] & 0b01111111) == 0b1111010)
    {
        // Address
        this->Type = UserDataType_Address;
    }
    else
    {
		Serial1.println("Unknown VIF: 0x");
		Serial1.println(VIF[0], HEX);
    }
}



