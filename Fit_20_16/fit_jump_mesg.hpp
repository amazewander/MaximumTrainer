////////////////////////////////////////////////////////////////////////////////
// The following FIT Protocol software provided may be used with FIT protocol
// devices only and remains the copyrighted property of Garmin Canada Inc.
// The software is being provided on an "as-is" basis and as an accommodation,
// and therefore all warranties, representations, or guarantees of any kind
// (whether express, implied or statutory) including, without limitation,
// warranties of merchantability, non-infringement, or fitness for a particular
// purpose, are specifically disclaimed.
//
// Copyright 2019 Garmin Canada Inc.
////////////////////////////////////////////////////////////////////////////////
// ****WARNING****  This file is auto-generated!  Do NOT edit this file.
// Profile Version = 21.20Release
// Tag = production/akw/21.20.00-0-g5907bff
////////////////////////////////////////////////////////////////////////////////


#if !defined(FIT_JUMP_MESG_HPP)
#define FIT_JUMP_MESG_HPP

#include "fit_mesg.hpp"

namespace fit
{

class JumpMesg : public Mesg
{
public:
    class FieldDefNum final
    {
    public:
       static const FIT_UINT8 Timestamp = 253;
       static const FIT_UINT8 Distance = 0;
       static const FIT_UINT8 Height = 1;
       static const FIT_UINT8 Rotations = 2;
       static const FIT_UINT8 HangTime = 3;
       static const FIT_UINT8 Score = 4;
       static const FIT_UINT8 PositionLat = 5;
       static const FIT_UINT8 PositionLong = 6;
       static const FIT_UINT8 Speed = 7;
       static const FIT_UINT8 EnhancedSpeed = 8;
       static const FIT_UINT8 Invalid = FIT_FIELD_NUM_INVALID;
    };

    JumpMesg(void) : Mesg(Profile::MESG_JUMP)
    {
    }

    JumpMesg(const Mesg &mesg) : Mesg(mesg)
    {
    }

    ///////////////////////////////////////////////////////////////////////
    // Checks the validity of timestamp field
    // Returns FIT_TRUE if field is valid
    ///////////////////////////////////////////////////////////////////////
    FIT_BOOL IsTimestampValid() const
    {
        const Field* field = GetField(253);
        if( FIT_NULL == field )
        {
            return FIT_FALSE;
        }

        return field->IsValueValid();
    }

    ///////////////////////////////////////////////////////////////////////
    // Returns timestamp field
    // Units: s
    ///////////////////////////////////////////////////////////////////////
    FIT_DATE_TIME GetTimestamp(void) const
    {
        return GetFieldUINT32Value(253, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Set timestamp field
    // Units: s
    ///////////////////////////////////////////////////////////////////////
    void SetTimestamp(FIT_DATE_TIME timestamp)
    {
        SetFieldUINT32Value(253, timestamp, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Checks the validity of distance field
    // Returns FIT_TRUE if field is valid
    ///////////////////////////////////////////////////////////////////////
    FIT_BOOL IsDistanceValid() const
    {
        const Field* field = GetField(0);
        if( FIT_NULL == field )
        {
            return FIT_FALSE;
        }

        return field->IsValueValid();
    }

    ///////////////////////////////////////////////////////////////////////
    // Returns distance field
    // Units: m
    ///////////////////////////////////////////////////////////////////////
    FIT_FLOAT32 GetDistance(void) const
    {
        return GetFieldFLOAT32Value(0, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Set distance field
    // Units: m
    ///////////////////////////////////////////////////////////////////////
    void SetDistance(FIT_FLOAT32 distance)
    {
        SetFieldFLOAT32Value(0, distance, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Checks the validity of height field
    // Returns FIT_TRUE if field is valid
    ///////////////////////////////////////////////////////////////////////
    FIT_BOOL IsHeightValid() const
    {
        const Field* field = GetField(1);
        if( FIT_NULL == field )
        {
            return FIT_FALSE;
        }

        return field->IsValueValid();
    }

    ///////////////////////////////////////////////////////////////////////
    // Returns height field
    // Units: m
    ///////////////////////////////////////////////////////////////////////
    FIT_FLOAT32 GetHeight(void) const
    {
        return GetFieldFLOAT32Value(1, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Set height field
    // Units: m
    ///////////////////////////////////////////////////////////////////////
    void SetHeight(FIT_FLOAT32 height)
    {
        SetFieldFLOAT32Value(1, height, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Checks the validity of rotations field
    // Returns FIT_TRUE if field is valid
    ///////////////////////////////////////////////////////////////////////
    FIT_BOOL IsRotationsValid() const
    {
        const Field* field = GetField(2);
        if( FIT_NULL == field )
        {
            return FIT_FALSE;
        }

        return field->IsValueValid();
    }

    ///////////////////////////////////////////////////////////////////////
    // Returns rotations field
    ///////////////////////////////////////////////////////////////////////
    FIT_UINT8 GetRotations(void) const
    {
        return GetFieldUINT8Value(2, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Set rotations field
    ///////////////////////////////////////////////////////////////////////
    void SetRotations(FIT_UINT8 rotations)
    {
        SetFieldUINT8Value(2, rotations, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Checks the validity of hang_time field
    // Returns FIT_TRUE if field is valid
    ///////////////////////////////////////////////////////////////////////
    FIT_BOOL IsHangTimeValid() const
    {
        const Field* field = GetField(3);
        if( FIT_NULL == field )
        {
            return FIT_FALSE;
        }

        return field->IsValueValid();
    }

    ///////////////////////////////////////////////////////////////////////
    // Returns hang_time field
    // Units: s
    ///////////////////////////////////////////////////////////////////////
    FIT_FLOAT32 GetHangTime(void) const
    {
        return GetFieldFLOAT32Value(3, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Set hang_time field
    // Units: s
    ///////////////////////////////////////////////////////////////////////
    void SetHangTime(FIT_FLOAT32 hangTime)
    {
        SetFieldFLOAT32Value(3, hangTime, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Checks the validity of score field
    // Returns FIT_TRUE if field is valid
    ///////////////////////////////////////////////////////////////////////
    FIT_BOOL IsScoreValid() const
    {
        const Field* field = GetField(4);
        if( FIT_NULL == field )
        {
            return FIT_FALSE;
        }

        return field->IsValueValid();
    }

    ///////////////////////////////////////////////////////////////////////
    // Returns score field
    // Comment: A score for a jump calculated based on hang time, rotations, and distance.
    ///////////////////////////////////////////////////////////////////////
    FIT_FLOAT32 GetScore(void) const
    {
        return GetFieldFLOAT32Value(4, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Set score field
    // Comment: A score for a jump calculated based on hang time, rotations, and distance.
    ///////////////////////////////////////////////////////////////////////
    void SetScore(FIT_FLOAT32 score)
    {
        SetFieldFLOAT32Value(4, score, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Checks the validity of position_lat field
    // Returns FIT_TRUE if field is valid
    ///////////////////////////////////////////////////////////////////////
    FIT_BOOL IsPositionLatValid() const
    {
        const Field* field = GetField(5);
        if( FIT_NULL == field )
        {
            return FIT_FALSE;
        }

        return field->IsValueValid();
    }

    ///////////////////////////////////////////////////////////////////////
    // Returns position_lat field
    // Units: semicircles
    ///////////////////////////////////////////////////////////////////////
    FIT_SINT32 GetPositionLat(void) const
    {
        return GetFieldSINT32Value(5, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Set position_lat field
    // Units: semicircles
    ///////////////////////////////////////////////////////////////////////
    void SetPositionLat(FIT_SINT32 positionLat)
    {
        SetFieldSINT32Value(5, positionLat, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Checks the validity of position_long field
    // Returns FIT_TRUE if field is valid
    ///////////////////////////////////////////////////////////////////////
    FIT_BOOL IsPositionLongValid() const
    {
        const Field* field = GetField(6);
        if( FIT_NULL == field )
        {
            return FIT_FALSE;
        }

        return field->IsValueValid();
    }

    ///////////////////////////////////////////////////////////////////////
    // Returns position_long field
    // Units: semicircles
    ///////////////////////////////////////////////////////////////////////
    FIT_SINT32 GetPositionLong(void) const
    {
        return GetFieldSINT32Value(6, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Set position_long field
    // Units: semicircles
    ///////////////////////////////////////////////////////////////////////
    void SetPositionLong(FIT_SINT32 positionLong)
    {
        SetFieldSINT32Value(6, positionLong, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Checks the validity of speed field
    // Returns FIT_TRUE if field is valid
    ///////////////////////////////////////////////////////////////////////
    FIT_BOOL IsSpeedValid() const
    {
        const Field* field = GetField(7);
        if( FIT_NULL == field )
        {
            return FIT_FALSE;
        }

        return field->IsValueValid();
    }

    ///////////////////////////////////////////////////////////////////////
    // Returns speed field
    // Units: m/s
    ///////////////////////////////////////////////////////////////////////
    FIT_FLOAT32 GetSpeed(void) const
    {
        return GetFieldFLOAT32Value(7, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Set speed field
    // Units: m/s
    ///////////////////////////////////////////////////////////////////////
    void SetSpeed(FIT_FLOAT32 speed)
    {
        SetFieldFLOAT32Value(7, speed, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Checks the validity of enhanced_speed field
    // Returns FIT_TRUE if field is valid
    ///////////////////////////////////////////////////////////////////////
    FIT_BOOL IsEnhancedSpeedValid() const
    {
        const Field* field = GetField(8);
        if( FIT_NULL == field )
        {
            return FIT_FALSE;
        }

        return field->IsValueValid();
    }

    ///////////////////////////////////////////////////////////////////////
    // Returns enhanced_speed field
    // Units: m/s
    ///////////////////////////////////////////////////////////////////////
    FIT_FLOAT32 GetEnhancedSpeed(void) const
    {
        return GetFieldFLOAT32Value(8, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

    ///////////////////////////////////////////////////////////////////////
    // Set enhanced_speed field
    // Units: m/s
    ///////////////////////////////////////////////////////////////////////
    void SetEnhancedSpeed(FIT_FLOAT32 enhancedSpeed)
    {
        SetFieldFLOAT32Value(8, enhancedSpeed, 0, FIT_SUBFIELD_INDEX_MAIN_FIELD);
    }

};

} // namespace fit

#endif // !defined(FIT_JUMP_MESG_HPP)
