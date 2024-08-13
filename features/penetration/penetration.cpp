// taken from here as im too lazy https://yougame.biz/threads/324402/#post-3117216

#include "../../includes.h"

static ConVar* ff_damage_reduction_bullets;
static ConVar* ff_damage_bullet_penetration;
static ConVar* mp_damage_scale_ct_head;
static ConVar* mp_damage_scale_ct_body;
static ConVar* mp_damage_scale_t_head;
static ConVar* mp_damage_scale_t_body;

inline float DistanceToRay( const vec3_t& pos, const vec3_t& rayStart, const vec3_t& rayEnd, float* along = NULL, vec3_t* pointOnRay = NULL ) {

    const vec3_t to = pos - rayStart;
    vec3_t dir = rayEnd - rayStart;
    const float length = dir.normalize( );
    const float rangeAlong = dir.dot( to );

    if ( along )
        *along = rangeAlong;

    float range;

    if ( rangeAlong < 0.0f ) {
        range = -( pos - rayStart ).length( );

        if ( pointOnRay )
            *pointOnRay = rayStart;
    }
    else if ( rangeAlong > length ) {
        range = -( pos - rayEnd ).length( );

        if ( pointOnRay )
            *pointOnRay = rayEnd;
    }
    else {
        const vec3_t onRay = rayStart + rangeAlong * dir;
        range = ( pos - onRay ).length( );

        if ( pointOnRay )
            *pointOnRay = onRay;
    }

    return range;
}

void UTIL_ClipTraceToPlayers( const vec3_t& vecAbsStart, const vec3_t& vecAbsEnd, unsigned int mask, ITraceFilter* filter, CGameTrace* tr, Player* pVictim = nullptr ) {

    CGameTrace playerTrace;
    const Ray ray( vecAbsStart, vecAbsEnd );
    float smallestFraction = tr->m_fraction;
    const float maxRange = 60.0f;

    for ( int i = 1; i <= g_csgo.m_globals->m_max_clients; ++i ) {
        Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

        if ( !player || !player->alive( ) )
            continue;

        if ( player->dormant( ) )
            continue;

        if ( pVictim && pVictim->index( ) != i )
            continue;

        if ( filter && filter->ShouldHitEntity( player, mask ) == false )
            continue;

        const float range = DistanceToRay( player->WorldSpaceCenter( ), vecAbsStart, vecAbsEnd );
        if ( range < 0.0f || range > maxRange )
            continue;

        g_csgo.m_engine_trace->ClipRayToEntity( ray, mask | CONTENTS_HITBOX, player, &playerTrace );
        if ( playerTrace.m_fraction < smallestFraction ) {
            *tr = playerTrace;
            smallestFraction = playerTrace.m_fraction;
        }
    }
}

inline void UTIL_TraceLineIgnoreTwoEntities( const vec3_t& vecAbsStart, const vec3_t& vecAbsEnd, unsigned int mask, const Entity* ignore, const Entity* ignore2, int collisionGroup, CGameTrace* ptr ) {
    CTraceFilterSkipTwoEntities traceFilter( ignore, ignore2, collisionGroup );
    g_csgo.m_engine_trace->TraceRay( Ray( vecAbsStart, vecAbsEnd ), mask, &traceFilter, ptr );
}

inline void UTIL_TraceLine( const vec3_t& vecAbsStart, const vec3_t& vecAbsEnd, unsigned int mask, const Entity* ignore, int collisionGroup, CGameTrace* ptr ) {
    CTraceFilterSimple traceFilter( ignore, collisionGroup );
    g_csgo.m_engine_trace->TraceRay( Ray( vecAbsStart, vecAbsEnd ), mask, &traceFilter, ptr );
}

inline void UTIL_TraceLine( const vec3_t& vecAbsStart, const vec3_t& vecAbsEnd, unsigned int mask, CTraceFilter* pFilter, CGameTrace* ptr ) {
    g_csgo.m_engine_trace->TraceRay( Ray( vecAbsStart, vecAbsEnd ), mask, pFilter, ptr );
}

static bool TraceToExit( vec3_t start, vec3_t dir, vec3_t& end, CGameTrace& trEnter, CGameTrace& trExit, float flStepSize, float flMaxDistance ) {
    float flDistance = 0;
    vec3_t last = start;
    int nStartContents = 0;

    while ( flDistance <= flMaxDistance ) {
        flDistance += flStepSize;

        end = start + ( flDistance * dir );

        vec3_t vecTrEnd = end - ( flStepSize * dir );
        const int nCurrentContents = g_csgo.m_engine_trace->GetPointContents_WorldOnly( end, CS_MASK_SHOOT | CONTENTS_HITBOX );

        if ( nStartContents == 0 )
            nStartContents = nCurrentContents;

        if ( ( nCurrentContents & CS_MASK_SHOOT ) == 0 || ( ( nCurrentContents & CONTENTS_HITBOX ) && nStartContents != nCurrentContents ) ) {
            // this gets a bit more complicated and expensive when we have to deal with displacements
            UTIL_TraceLine( end, vecTrEnd, CS_MASK_SHOOT | CONTENTS_HITBOX, NULL, &trExit );

            // we exited the wall into a player's hitbox
            if ( trExit.m_startsolid == true && ( trExit.m_surface.m_flags & SURF_HITBOX ) ) {
                // do another trace, but skip the player to get the actual exit surface
                UTIL_TraceLine( end, start, CS_MASK_SHOOT, trExit.m_entity, COLLISION_GROUP_NONE, &trExit );
                if ( trExit.hit( ) && trExit.m_startsolid == false ) {
                    end = trExit.m_endpos;
                    return true;
                }
            }
            else if ( trExit.hit( ) && trExit.m_startsolid == false ) {
                const bool bStartIsNodraw = !!( trEnter.m_surface.m_flags & SURF_NODRAW );
                const bool bExitIsNodraw = !!( trExit.m_surface.m_flags & SURF_NODRAW );
                if ( bExitIsNodraw && game::IsBreakable( trExit.m_entity ) && game::IsBreakable( trEnter.m_entity ) ) {
                    // we have a case where we have a breakable object, but the mapper put a nodraw on the backside
                    end = trExit.m_endpos;
                    return true;
                }
                else if ( bExitIsNodraw == false || ( bStartIsNodraw && bExitIsNodraw ) ) // exit nodraw is only valid if our entrace is also nodraw
                {
                    const vec3_t vecNormal = trExit.m_plane.m_normal;
                    const float flDot = dir.dot( vecNormal );
                    if ( flDot <= 1.0f ) {
                        // get the real end pos
                        end = end - ( ( flStepSize * trExit.m_fraction ) * dir );
                        return true;
                    }
                }
            }
            else if ( trEnter.m_entity
                      && trEnter.m_entity != g_csgo.m_entlist->GetClientEntity( 0 )
                      && game::IsBreakable( trEnter.m_entity ) ) {
                      // if we hit a breakable, make the assumption that we broke it if we can't find an exit (hopefully..)
                      // fake the end pos
                trExit = trEnter;
                trExit.m_endpos = start + ( 1.0f * dir );
                return true;
            }
        }
    }

    return false;
}

bool Player::IsArmored( int nHitGroup ) {
    if ( !this )
        return false;

    bool bApplyArmor = false;

    if ( m_ArmorValue( ) > 0 ) {
        switch ( nHitGroup ) {
            case HITGROUP_GENERIC:
            case HITGROUP_CHEST:
            case HITGROUP_STOMACH:
            case HITGROUP_LEFTARM:
            case HITGROUP_RIGHTARM:
                bApplyArmor = true;
                break;
            case HITGROUP_HEAD:
                if ( m_bHasHelmet( ) ) {
                    bApplyArmor = true;
                }
                break;
            default:
                break;
        }
    }

    return bApplyArmor;
}

float Player::ScaleDamage( float flWpnArmorRatio, int group, float fDamage ) {
    if ( !this )
        return 0.0f;

    float flArmorBonus = 0.5f;
    float fDamageToHealth = fDamage;
    float fDamageToArmor = 0;
    float fHeavyArmorBonus = 1.0f;
    float flArmorRatio = 0.5f;
    flArmorRatio *= flWpnArmorRatio;

    if ( !mp_damage_scale_ct_body )
        mp_damage_scale_ct_body = g_csgo.m_cvar->FindVar( HASH( "mp_damage_scale_ct_body" ) );

    if ( !mp_damage_scale_ct_head )
        mp_damage_scale_ct_head = g_csgo.m_cvar->FindVar( HASH( "mp_damage_scale_ct_head" ) );

    if ( !mp_damage_scale_t_body )
        mp_damage_scale_t_body = g_csgo.m_cvar->FindVar( HASH( "mp_damage_scale_t_body" ) );

    if ( !mp_damage_scale_t_head )
        mp_damage_scale_t_head = g_csgo.m_cvar->FindVar( HASH( "mp_damage_scale_t_head" ) );

    float flBodyDamageScale = ( m_iTeamNum( ) == TEAM_COUNTERTERRORISTS ) ? mp_damage_scale_ct_body->GetFloat( ) : mp_damage_scale_t_body->GetFloat( );
    float flHeadDamageScale = ( m_iTeamNum( ) == TEAM_COUNTERTERRORISTS ) ? mp_damage_scale_ct_head->GetFloat( ) : mp_damage_scale_t_head->GetFloat( );

    if ( m_bHasHeavyArmor( ) ) {
        flArmorRatio *= 0.5f;
        flArmorBonus = 0.33f;
        fHeavyArmorBonus = 0.33f;

        // heavy armor reduces headshot damage by have of what it is, so it does x2 instead of x4
        flHeadDamageScale = flHeadDamageScale * 0.5f;
    }

    switch ( group ) {
        case HITGROUP_GENERIC:
            break;

        case HITGROUP_HEAD:
            fDamage *= 4.f;
            fDamage *= flHeadDamageScale;
            break;

        case HITGROUP_CHEST:
            fDamage *= 1.f;
            fDamage *= flBodyDamageScale;
            break;

        case HITGROUP_STOMACH:
            fDamage *= 1.25f;
            fDamage *= flBodyDamageScale;
            break;

        case HITGROUP_LEFTARM:
        case HITGROUP_RIGHTARM:
            fDamage *= 1.0f;
            fDamage *= flBodyDamageScale;
            break;

        case HITGROUP_LEFTLEG:
        case HITGROUP_RIGHTLEG:
            fDamage *= 0.75f;
            fDamage *= flBodyDamageScale;
            break;

        default:
            break;
    }

    // deal with Armour
    if ( m_ArmorValue( ) > 0.f && IsArmored( group ) ) {
        fDamageToHealth = fDamage * flArmorRatio;
        fDamageToArmor = ( fDamage - fDamageToHealth ) * ( flArmorBonus * fHeavyArmorBonus );

        const int armorValue = m_ArmorValue( );

        // Does this use more armor than we have?
        if ( fDamageToArmor > armorValue )
            fDamageToHealth = fDamage - armorValue / flArmorBonus;

        return fDamageToHealth;
    }

    return fDamage;
}

bool penetration::run( PenetrationInput_t* in, PenetrationOutput_t* out ) {
    int              iPenCount{ 4 };
    vec3_t          vecStart, dir, end, pen_end;
    ang_t          angAimDir;
    CGameTrace      trace, exit_trace;
    Weapon* weapon;
    WeaponInfo* weapon_info;

    // if we are tracing from our local player perspective.
    if ( in->m_from->m_bIsLocalPlayer( ) ) {
        weapon = g_cl.m_weapon;
        weapon_info = g_cl.m_weapon_info;
        vecStart = g_cl.m_shoot_pos;
    }

    // not local player.
    else {
        weapon = in->m_from->GetActiveWeapon( );
        if ( !weapon )
            return false;

        // get weapon info.
        weapon_info = weapon->GetWpnData( );
        if ( !weapon_info )
            return false;

        // set trace start.
        vecStart = in->m_from->GetShootPosition( );
    }

    // calculate aim dir.
    math::VectorAngles( in->m_pos - g_cl.m_shoot_pos, angAimDir );

    // weapon damage.
    int iDamage = weapon_info->m_damage;

    // get damage and hitgroup for our shot.
    const int iHitgroup = in->m_from->FireBullet( vecStart,
                                                  angAimDir,
                                                  weapon_info->m_armor_ratio,
                                                  weapon_info->m_range,
                                                  weapon_info->m_penetration,
                                                  iPenCount,
                                                  iDamage,
                                                  weapon_info->m_range_modifier,
                                                  in->m_target,
                                                  trace );

    // we need to get a damage trace
    if ( in->m_target ) {
        if ( iHitgroup > HITGROUP_GENERIC ) {
            out->m_damage = iDamage;
            out->m_hitgroup = ( weapon->m_iItemDefinitionIndex( ) == ZEUS ) ? HITGROUP_GENERIC : iHitgroup;
            out->m_pen = iPenCount;
			out->m_target = in->m_target;

            if ( iDamage > in->m_target->m_iHealth( ) )
                return true;

            if ( iPenCount != 4 )
                return ( iDamage >= in->m_damage_pen );

            return ( iDamage >= in->m_damage );
        }

        out->m_damage = -1.f;
        out->m_hitgroup = -1;
        out->m_pen = iPenCount;
        return false;
    }
    // target for g_cl.m_pen_data will always be false/nullptr so for pen crosshair we need to trace a target
    else {
        out->m_pen = iPenCount;
        out->m_damage = iDamage;

        if ( iHitgroup > HITGROUP_GENERIC ) {
            out->m_hitgroup = ( weapon->m_iItemDefinitionIndex( ) == ZEUS ) ? HITGROUP_GENERIC : iHitgroup;
            out->m_target = trace.m_entity->as<Player*>( );

            if ( iDamage > trace.m_entity->as<Player*>( )->m_iHealth( ) )
                return true;

            if ( iPenCount != 4 )
                return ( iDamage >= in->m_damage_pen );

            return ( iDamage >= in->m_damage );
        }

		// note don't reset the damage values here, otherwise with pen crosshair it will just show as -1 isntead of the max damage our weapon can do :)
        return false;
    }

    return ( iDamage > 0.f );
}

int Player::FireBullet(
    vec3_t vecSrc,    // shooting postion
    const ang_t& shootAngles,  //shooting angle
    float flArmorRatio, // ratio for armor
    float flDistance, // max distance
    float flPenetration, // the power of the penetration
    int& nPenetrationCount,
    int& iDamage, // base damage
    float flRangeModifier, // damage range modifier
    Player* pVictim,
    CGameTrace& tr ) {
    if ( !this )
		return -1;

    // Check for player hitboxes extending outside their collision bounds
    constexpr float rayExtension = 40.0f;
    float fCurrentDamage = iDamage;   // damage of the bullet at it's current trajectory
    float flCurrentDistance = 0.0;  //distance that the bullet has traveled so far

    vec3_t vecDirShooting, vecRight, vecUp;
    math::AngleVectors( shootAngles, &vecDirShooting, &vecRight, &vecUp );

    // MIKETODO: put all the ammo parameters into a script file and allow for CS-specific params.
    const float flPenetrationPower = flPenetration;         // thickness of a wall that this bullet can penetrate
    constexpr float flPenetrationDistance = 3000.f;    // distance at which the bullet is capable of penetrating a wall
    float flDamageModifier = 0.5f;        // default modification of bullets power after they go through a wall.
    float flPenetrationModifier = 1.0f;

    vec3_t vecDir = vecDirShooting;
    vecDir.normalize( );

    int iHitgroup = -1;
    Player* lastPlayerHit = NULL;    // this includes players, bots, and hostages
    //CGameTrace tr; // main enter bullet trace

    while ( fCurrentDamage > 0.f ) {
        const vec3_t vecEnd = vecSrc + vecDir * ( flDistance - flCurrentDistance );

        UTIL_TraceLineIgnoreTwoEntities( vecSrc, vecEnd, CS_MASK_SHOOT | CONTENTS_HITBOX, this, lastPlayerHit, COLLISION_GROUP_NONE, &tr );
        {
            CTraceFilterSkipTwoEntities filter( this, lastPlayerHit, COLLISION_GROUP_NONE );
            UTIL_ClipTraceToPlayers( vecSrc, vecEnd + vecDir * rayExtension, CS_MASK_SHOOT | CONTENTS_HITBOX, &filter, &tr, pVictim );
        }

        if ( tr.m_fraction == 1.0f )
            break; // we didn't hit anything, stop tracing shoot

        /************* MATERIAL DETECTION ***********/
        surfacedata_t* pSurfaceData = g_csgo.m_phys_props->GetSurfaceData( tr.m_surface.m_surface_props );
        int iEnterMaterial = pSurfaceData->m_game.m_material;

        flPenetrationModifier = pSurfaceData->m_game.m_penetration_modifier;
        flDamageModifier = pSurfaceData->m_game.m_damage_modifier;

        bool hitGrate = ( tr.m_contents & CONTENTS_GRATE ) != 0;

        //calculate the damage based on the distance the bullet travelled.
        flCurrentDistance += tr.m_fraction * ( flDistance - flCurrentDistance );
        fCurrentDamage *= std::powf( flRangeModifier, ( flCurrentDistance / 500.f ) );

        // check if we hit player
        if ( tr.m_hitgroup > HITGROUP_GENERIC
             && tr.m_hitgroup < HITGROUP_GEAR
             && tr.m_entity
             && tr.m_entity->index( ) > 0
             && tr.m_entity->index( ) <= 64 ) {
             // NOTE: inaccurate if there's players inside of each other
            if ( !pVictim || tr.m_entity->index( ) == pVictim->index( ) ) {
                if ( tr.m_entity->as< Player* >( )->enemy( this ) ) {
                    lastPlayerHit = tr.m_entity->as< Player* >( );
                    iHitgroup = tr.m_hitgroup;
                    break;
                }
            }
        }

        // check if we reach penetration distance, no more penetrations after that
        // or if our modifyer is super low, just stop the bullet
        if ( ( flCurrentDistance > flPenetrationDistance && flPenetration > 0.f ) ||
             flPenetrationModifier < 0.1f ) {
             // Setting nPenetrationCount to zero prevents the bullet from penetrating object at max distance
             // and will no longer trace beyond the exit point, however "numPenetrationsInitiallyAllowedForThisBullet"
             // is saved off to allow correct determination whether the hit on the object at max distance had
             // *previously* penetrated anything or not. In case of a direct hit over 3000 units the saved off
             // value would be max penetrations value and will determine a direct hit and not a penetration hit.
             // However it is important that all tracing further stops past this point (as the code does at
             // the time of writing) because otherwise next trace will think that 4 penetrations have already
             // occurred.
            nPenetrationCount = 0;
            break;
        }

        // [dkorus] note: values are changed inside of HandleBulletPenetration
        bool bulletStopped = HandleBulletPenetration( flPenetration, iEnterMaterial, hitGrate, tr, vecDir, pSurfaceData, flPenetrationModifier,
                                                      flDamageModifier, flPenetrationPower, nPenetrationCount, vecSrc, flDistance,
                                                      flCurrentDistance, fCurrentDamage );

                                                  // [dkorus] bulletStopped is true if the bullet can no longer continue penetrating materials
        if ( bulletStopped )
            break;
    }

    // report weapon damage.
    iDamage = std::max( 0, static_cast< int >( tr.m_entity->as< Player* >( )->ScaleDamage( flArmorRatio, -1, fCurrentDamage ) ) );

    // hit a player.
    if ( lastPlayerHit != NULL ) {
        // report damage and hitgroup.
        iDamage = std::max( 0, static_cast< int >( lastPlayerHit->ScaleDamage( flArmorRatio, iHitgroup, fCurrentDamage ) ) );
        return iHitgroup;
    }

    // we hit nothing or our bullet got stopped.
    return -1;
}

// [dkorus] helper for FireBullet
//            changes iPenetration to updated value
//            returns TRUE if we should stop processing more hits after this one
//            returns FALSE if we can continue processing
bool Player::HandleBulletPenetration( float& flPenetration,
                                      int& iEnterMaterial,
                                      bool& hitGrate,
                                      CGameTrace& tr,
                                      vec3_t& vecDir,
                                      surfacedata_t* pSurfaceData,
                                      float flPenetrationModifier,
                                      float flDamageModifier,
                                      float flPenetrationPower,
                                      int& nPenetrationCount,
                                      vec3_t& vecSrc,
                                      float flDistance,
                                      float flCurrentDistance,
                                      float& fCurrentDamage ) {
    if ( !this )
        return false;

    // NOTE: not thread-safe
    if ( !ff_damage_reduction_bullets )
        ff_damage_reduction_bullets = g_csgo.m_cvar->FindVar( HASH( "ff_damage_reduction_bullets" ) );

    if ( !ff_damage_bullet_penetration )
        ff_damage_bullet_penetration = g_csgo.m_cvar->FindVar( HASH( "ff_damage_bullet_penetration" ) );

    bool bIsNodraw = !!( tr.m_surface.m_flags & SURF_NODRAW );
    bool bFailedPenetrate = false;

    // check if bullet can penetrarte another entity
    if ( nPenetrationCount == 0 && !hitGrate && !bIsNodraw
         && iEnterMaterial != CHAR_TEX_GLASS && iEnterMaterial != CHAR_TEX_GRATE )
        bFailedPenetrate = true; // no, stop

    // If we hit a grate with iPenetration == 0, stop on the next thing we hit
    if ( flPenetration <= 0 || nPenetrationCount <= 0 )
        bFailedPenetrate = true;

    vec3_t penetrationEnd;

    if ( bFailedPenetrate == true )
        return true;

    // find exact penetration exit
    CGameTrace exitTr;
    if ( !TraceToExit( tr.m_endpos, vecDir, penetrationEnd, tr, exitTr, 4, 90.f ) ) {
        // ended in solid
        if ( ( g_csgo.m_engine_trace->GetPointContents_WorldOnly( tr.m_endpos, CS_MASK_SHOOT ) & CS_MASK_SHOOT ) == 0 ) {
            return true;
        }
    }

    // get material at exit point
    surfacedata_t* pExitSurfaceData = g_csgo.m_phys_props->GetSurfaceData( exitTr.m_surface.m_surface_props );
    int iExitMaterial = pExitSurfaceData->m_game.m_material;

    // percent of total damage lost automatically on impacting a surface
    float flDamLostPercent = 0.16f;

    // since some railings in de_inferno are CONTENTS_GRATE but CHAR_TEX_CONCRETE, we'll trust the
    // CONTENTS_GRATE and use a high damage modifier.
    if ( hitGrate || bIsNodraw || iEnterMaterial == CHAR_TEX_GLASS || iEnterMaterial == CHAR_TEX_GRATE ) {
        // If we're a concrete grate (TOOLS/TOOLSINVISIBLE texture) allow more penetrating power.
        if ( iEnterMaterial == CHAR_TEX_GLASS || iEnterMaterial == CHAR_TEX_GRATE ) {
            flPenetrationModifier = 3.0f;
            flDamLostPercent = 0.05f;
        }
        else
            flPenetrationModifier = 1.0f;

        flDamageModifier = 0.99f;
    }
    // NOTE: the cvars aren't thread-safe
    else if ( iEnterMaterial == CHAR_TEX_FLESH && ff_damage_reduction_bullets->GetFloat( ) == 0
              && tr.m_entity && tr.m_entity->IsPlayer( ) && tr.m_entity->m_iTeamNum( ) == m_iTeamNum( ) ) {
        if ( ff_damage_bullet_penetration->GetFloat( ) == 0.f ) {
            // don't allow penetrating players when FF is off
            flPenetrationModifier = 0.f;
            return true;
        }

        flPenetrationModifier = ff_damage_bullet_penetration->GetFloat( );
        flDamageModifier = ff_damage_bullet_penetration->GetFloat( );
    }
    else {
        // check the exit material and average the exit and entrace values
        float flExitPenetrationModifier = pExitSurfaceData->m_game.m_penetration_modifier;
        float flExitDamageModifier = pExitSurfaceData->m_game.m_damage_modifier;
        flPenetrationModifier = ( flPenetrationModifier + flExitPenetrationModifier ) / 2.f;
        flDamageModifier = ( flDamageModifier + flExitDamageModifier ) / 2.f;
    }

    // if enter & exit point is wood we assume this is
    // a hollow crate and give a penetration bonus
    if ( iEnterMaterial == iExitMaterial ) {
        if ( iExitMaterial == CHAR_TEX_WOOD || iExitMaterial == CHAR_TEX_CARDBOARD ) {
            flPenetrationModifier = 3;
        }
        else if ( iExitMaterial == CHAR_TEX_PLASTIC ) {
            flPenetrationModifier = 2;
        }
    }

    // [side change] used squared length, because game wastefully converts it back to squared
    const float flTraceDistanceSqr = ( exitTr.m_endpos - tr.m_endpos ).length_sqr( );

    // penetration modifier
    const float flModifier = ( flPenetrationModifier > 0.0f ? 1.0f / flPenetrationModifier : 0.0f );

    // this calculates how much damage we've lost depending on thickness of the wall, our penetration, damage, and the modifiers set earlier
    const float flLostDamage = ( fCurrentDamage * flDamLostPercent + ( flPenetration > 0.0f ? 3.75f / flPenetration : 0.0f ) * ( flModifier * 3.0f ) ) + ( ( flModifier * flTraceDistanceSqr ) / 24.0f );

    // reduce damage power each time we hit something other than a grate
    fCurrentDamage -= std::max( 0.f, flLostDamage );

    if ( fCurrentDamage < 1.f )
        return true;

    flCurrentDistance += flTraceDistanceSqr;
    vecSrc = exitTr.m_endpos;
    flDistance = ( flDistance - flCurrentDistance ) * 0.5f;

    nPenetrationCount--;
    return false;
}