#if defined _INC_PROJECTILE
    #endinput
#endif
#define _INC_PROJECTILE

#include <colandreas>

#include <YSI_Coding\y_hooks>
#include <YSI_Data\y_iterate>

/**
 * # Functions
 */

forward Projectile:CreateProjectile(Float:x, Float:y, Float:z, Float:velocityX, Float:velocityY, Float:velocityZ, Float:rotationX = 0.0, Float:rotationY = 0.0, Float:rotationZ = 0.0, Float:sphereColRadius = 1.0, Float:groundFriction = 5.0, Float:collisionFriction = 0.2, Float:airResistance = 0.5, Float:gravity = 10.0, Float:playerColRadius = 0.8, bool:collideSimulation = false, Float:mass = 1.0);
forward bool:IsValidProjectile(Projectile:projid);
forward bool:DestroyProjectile(Projectile:projid);
forward bool:GetProjectilePos(Projectile:projid, &Float:x, &Float:y, &Float:z);
forward bool:GetProjectileRot(Projectile:projid, &Float:rotationX, &Float:rotationY, &Float:rotationZ);
forward bool:GetProjectileVel(Projectile:projid, &Float:velocityX, &Float:velocityY, &Float:velocityZ);
forward bool:UpdateProjectileVel(Projectile:projid, Float:velocityX, Float:velocityY, Float:velocityZ);

/**
 * # Events
 */

forward OnProjectileUpdate(projid);
forward OnProjectileStop(projid, Float:x, Float:y, Float:z);
forward OnProjectileCollide(projid, type, Float:x, Float:y, Float:z, extraid);

/**
 * # Header
 */

#if !defined MAX_PROJECTILES
    #define MAX_PROJECTILES \
        (Projectile:64)
#endif

#if !defined PROJECTILE_TIMER_INTERVAL
    #define PROJECTILE_TIMER_INTERVAL \
        (20)
#endif

#define INVALID_PROJECTILE_ID (Projectile:-1)

enum {
    PROJECTILE_COLLIDE_GROUND,
    PROJECTILE_COLLIDE_CIELING,
    PROJECTILE_COLLIDE_SIMULATION,
    PROJECTILE_COLLIDE_OBJECT,
    PROJECTILE_COLLIDE_PLAYER
};

enum E_PROJECTILE_DATA {
    Float:E_PROJECTILE_X,
    Float:E_PROJECTILE_Y,
    Float:E_PROJECTILE_Z,
    Float:E_PROJECTILE_ROT_X,
    Float:E_PROJECTILE_ROT_Y,
    Float:E_PROJECTILE_ROT_Z,
    Float:E_PROJECTILE_VEL_X,
    Float:E_PROJECTILE_VEL_Y,
    Float:E_PROJECTILE_VEL_Z,
    Float:E_PROJECTILE_GRAVITY,
    Float:E_PROJECTILE_GROUND_FRICTION,
    Float:E_PROJECTILE_COLLISION_FRICTION,
    Float:E_PROJECTILE_AIR_RESISTANCE,
    Float:E_PROJECTILE_OBJECT_COL_RADIUS,
    Float:E_PROJECTILE_PLAYER_COL_RADIUS,
    Float:E_PROJECTILE_MASS,
    bool:E_PROJECTILE_SIMULATION_COLLISION
};

static
    projectileData[MAX_PROJECTILES][E_PROJECTILE_DATA],
    projectileTimer
;

const static
    PROJECTILE_ITER_SIZE = _:MAX_PROJECTILES
;

new
    Iterator:Projectile<Projectile:PROJECTILE_ITER_SIZE>
;

/**
 * # Calls
 */

forward OnProjectileUpdate(Projectile:projid);
forward OnProjectileStop(Projectile:projid, Float:x, Float:y, Float:z);
forward OnProjectileCollide(Projectile:projid, collision, Float:x, Float:y, Float:z, extraid);

/**
 * # Internal
 */

static Internal_VonU(Float:vx, Float:vy, Float:vz, Float:ux, Float:uy, Float:uz, &Float:x, &Float:y, &Float:z) {
    new const
        Float:fac = ((vx * ux) + (vy * uy) + (vz * uz)) / ((ux * ux) + (uy * uy) + (uz * uz))
    ;

    x = ux * fac;
    y = uy * fac;
    z = uz * fac;
}

/**
 * # External
 */

stock Projectile:CreateProjectile(Float:x, Float:y, Float:z, Float:velocityX, Float:velocityY, Float:velocityZ, Float:rotationX = 0.0, Float:rotationY = 0.0, Float:rotationZ = 0.0, Float:sphereColRadius = 1.0, Float:groundFriction = 5.0, Float:collisionFriction = 0.2, Float:airResistance = 0.5, Float:gravity = 10.0, Float:playerColRadius = 0.8, bool:collideSimulation = false, Float:mass = 1.0) {
    if (sphereColRadius <= 0.0) {
        return INVALID_PROJECTILE_ID;
    }

    if (playerColRadius < 0.0) {
        return INVALID_PROJECTILE_ID;
    }

    if (!IsValidTimer(projectileTimer)) {
        projectileTimer = SetTimer("Internal_OnProjectilesUpdate", PROJECTILE_TIMER_INTERVAL, true);
    }

    new const
        Projectile:projid = Projectile:Iter_Alloc(Projectile)
    ;

    if (_:projid == INVALID_ITERATOR_SLOT) {
        return INVALID_PROJECTILE_ID;
    }

    projectileData[projid][E_PROJECTILE_X] = x;
     projectileData[projid][E_PROJECTILE_Y] = y;
     projectileData[projid][E_PROJECTILE_Z] = z;
    projectileData[projid][E_PROJECTILE_VEL_X] = velocityX;
    projectileData[projid][E_PROJECTILE_VEL_Y] = velocityY;
    projectileData[projid][E_PROJECTILE_VEL_Z] = velocityZ;
      projectileData[projid][E_PROJECTILE_ROT_X] = rotationX;
    projectileData[projid][E_PROJECTILE_ROT_Y] = rotationY;
    projectileData[projid][E_PROJECTILE_ROT_Z] = rotationZ;
    projectileData[projid][E_PROJECTILE_GRAVITY] = gravity;
    projectileData[projid][E_PROJECTILE_GROUND_FRICTION] = groundFriction;
    projectileData[projid][E_PROJECTILE_COLLISION_FRICTION] = collisionFriction;
    projectileData[projid][E_PROJECTILE_AIR_RESISTANCE] = airResistance;
    projectileData[projid][E_PROJECTILE_OBJECT_COL_RADIUS] = sphereColRadius;
    projectileData[projid][E_PROJECTILE_PLAYER_COL_RADIUS] = playerColRadius;
    projectileData[projid][E_PROJECTILE_MASS] = mass;
     projectileData[projid][E_PROJECTILE_SIMULATION_COLLISION] = collideSimulation;

    return projid;
}

stock bool:IsValidProjectile(Projectile:projid) {
    if (!(0 <= _:projid < PROJECTILE_ITER_SIZE)) {
        return false;
    }

    return Iter_Contains(Projectile, projid);
}

stock bool:DestroyProjectile(Projectile:projid) {
    if (!IsValidProjectile(projid)) {
        return false;
    }

    Iter_Remove(Projectile, projid);

    if (Iter_IsEmpty(Projectile)) {
        KillTimer(projectileTimer);
        projectileTimer = 0;
    }

    return true;
}

stock bool:GetProjectilePos(Projectile:projid, &Float:x, &Float:y, &Float:z) {
    if (!IsValidProjectile(projid)) {
        return false;
    }

    x = projectileData[projid][E_PROJECTILE_X];
    y = projectileData[projid][E_PROJECTILE_Y];
    z = projectileData[projid][E_PROJECTILE_Z];

    return true;
}

stock bool:GetProjectileRot(Projectile:projid, &Float:rotationX, &Float:rotationY, &Float:rotationZ) {
    if (!IsValidProjectile(projid)) {
        return false;
    }

    rotationX = projectileData[projid][E_PROJECTILE_ROT_X];
    rotationY = projectileData[projid][E_PROJECTILE_ROT_Y];
    rotationZ = projectileData[projid][E_PROJECTILE_ROT_Z];

    return true;
}

stock bool:GetProjectileVel(Projectile:projid, &Float:velocityX, &Float:velocityY, &Float:velocityZ) {
    if (!IsValidProjectile(projid)) {
        return false;
    }

    velocityX = projectileData[projid][E_PROJECTILE_VEL_X];
    velocityY = projectileData[projid][E_PROJECTILE_VEL_Y];
    velocityZ = projectileData[projid][E_PROJECTILE_VEL_Z];

    return true;
}

stock bool:UpdateProjectileVel(Projectile:projid, Float:velocityX, Float:velocityY, Float:velocityZ) {
    if (!IsValidProjectile(projid)) {
        return false;
    }

    projectileData[projid][E_PROJECTILE_VEL_X] = velocityX;
    projectileData[projid][E_PROJECTILE_VEL_Y] = velocityY;
    projectileData[projid][E_PROJECTILE_VEL_Z] = velocityZ;

    return true;
}

forward Internal_OnProjectilesUpdate();
public Internal_OnProjectilesUpdate() {
    new
        Float:unused,
        Float:new_x,
        Float:new_y,
        Float:new_z,
        Float:max_height,
        Float:min_height,
        Float:cx,
        Float:cy,
        Float:cz,
        Float:crx,
        Float:cry,
        extraid,
        Float:dx,
         Float:dy,
         Float:dz,
         Float:moveangle,
         Float:new_vx,
         Float:new_vy,
         Float:new_vz,
        Float:speed,
        bool:collision
    ;

    foreach (new Projectile:i : Projectile) {
        collision = false;

        // calculate next position after this timer step
        new_x = projectileData[i][E_PROJECTILE_X] + projectileData[i][E_PROJECTILE_VEL_X] * (PROJECTILE_TIMER_INTERVAL / 1000.0);
        new_y = projectileData[i][E_PROJECTILE_Y] + projectileData[i][E_PROJECTILE_VEL_Y] * (PROJECTILE_TIMER_INTERVAL / 1000.0);
        new_z = projectileData[i][E_PROJECTILE_Z] + projectileData[i][E_PROJECTILE_VEL_Z] * (PROJECTILE_TIMER_INTERVAL / 1000.0);

         // calculate minimum height the sphere can reach
        if (CA_RayCastLine(projectileData[i][E_PROJECTILE_X], projectileData[i][E_PROJECTILE_Y], projectileData[i][E_PROJECTILE_Z], new_x, new_y, new_z - 1000.0, unused, unused, min_height) != 0) {
            min_height += projectileData[i][E_PROJECTILE_OBJECT_COL_RADIUS];
        } else {
            min_height = projectileData[i][E_PROJECTILE_Z] - 1.0;
        }

         // calculate maximum height the sphere can reach
        if (CA_RayCastLine(projectileData[i][E_PROJECTILE_X], projectileData[i][E_PROJECTILE_Y], projectileData[i][E_PROJECTILE_Z], new_x, new_y, new_z + 1000.0, unused, unused, max_height) != 0) {
            if (max_height > min_height) {
                max_height -= projectileData[i][E_PROJECTILE_OBJECT_COL_RADIUS];
            } else {
                max_height = FLOAT_INFINITY;
            }
        } else {
            max_height = FLOAT_INFINITY;
        }

        // collision check with max-height possible of a projectile (cieling)
        if (new_z > max_height) {
            if (projectileData[i][E_PROJECTILE_VEL_Z] > 0) {
                projectileData[i][E_PROJECTILE_VEL_Z] = -projectileData[i][E_PROJECTILE_VEL_Z] * 0.8;
            }

            CallLocalFunction("OnProjectileCollide", "iiiiii", _:i, PROJECTILE_COLLIDE_CIELING, new_x, new_y, new_z, 0);

            new_z = max_height;
        }
        // collision check with min-height possible of a projectile (ground)
        else if (new_z < min_height) {
            if (projectileData[i][E_PROJECTILE_VEL_Z] < 0) {
                projectileData[i][E_PROJECTILE_VEL_Z] = -projectileData[i][E_PROJECTILE_VEL_Z] * 0.8;
            }

            CallLocalFunction("OnProjectileCollide", "iiiiii", _:i, PROJECTILE_COLLIDE_GROUND, new_x, new_y, new_z, 0);

            new_z = min_height;
        }

        // apply gravitation force (if gravity is set to non-zero)
        if (projectileData[i][E_PROJECTILE_GRAVITY] != 0.0) {
            if (projectileData[i][E_PROJECTILE_VEL_Z] > 0) {
                projectileData[i][E_PROJECTILE_VEL_Z] -= projectileData[i][E_PROJECTILE_GRAVITY] * (PROJECTILE_TIMER_INTERVAL / 1000.0);

                if (projectileData[i][E_PROJECTILE_VEL_Z] < 0) {
                    projectileData[i][E_PROJECTILE_VEL_Z] = 0;
                }
            } else {
                projectileData[i][E_PROJECTILE_VEL_Z] -= projectileData[i][E_PROJECTILE_GRAVITY] * (PROJECTILE_TIMER_INTERVAL / 1000.0);
            }
        }

        // collision check with another simulation/projectile
        if (projectileData[i][E_PROJECTILE_SIMULATION_COLLISION] == true) {
            foreach (new Projectile:x : Projectile) {
                if (x != i && projectileData[x][E_PROJECTILE_SIMULATION_COLLISION] == true) {
                    dx = projectileData[i][E_PROJECTILE_X] - projectileData[x][E_PROJECTILE_X];
                    dy = projectileData[i][E_PROJECTILE_Y] - projectileData[x][E_PROJECTILE_Y];
                    dz = projectileData[i][E_PROJECTILE_Z] - projectileData[x][E_PROJECTILE_Z];

                    if (((dx * dx) + (dy * dy) + (dz * dz)) < (projectileData[i][E_PROJECTILE_OBJECT_COL_RADIUS] * projectileData[x][E_PROJECTILE_OBJECT_COL_RADIUS])) {
                        // MAIN SIMULATION

                        new_vx = projectileData[i][E_PROJECTILE_VEL_X];
                        new_vy = projectileData[i][E_PROJECTILE_VEL_Y];
                        new_vz = projectileData[i][E_PROJECTILE_VEL_Z];

                        Internal_VonU(projectileData[i][E_PROJECTILE_VEL_X], projectileData[i][E_PROJECTILE_VEL_Y], projectileData[i][E_PROJECTILE_VEL_Z], dx, dy, dz, cx, cy, cz);
                        
                        new_vx -= cx;
                        new_vy -= cy;
                        new_vz -= cz;

                        Internal_VonU(projectileData[x][E_PROJECTILE_VEL_X], projectileData[x][E_PROJECTILE_VEL_Y], projectileData[x][E_PROJECTILE_VEL_Z], -dx, -dy, -dz, cx, cy, cz);
                        
                        cx = ((projectileData[i][E_PROJECTILE_MASS] - projectileData[x][E_PROJECTILE_MASS]) * projectileData[i][E_PROJECTILE_VEL_X] + 2 * projectileData[x][E_PROJECTILE_MASS] * cx) / (projectileData[i][E_PROJECTILE_MASS] + projectileData[x][E_PROJECTILE_MASS]);
                        cy = ((projectileData[i][E_PROJECTILE_MASS] - projectileData[x][E_PROJECTILE_MASS]) * projectileData[i][E_PROJECTILE_VEL_Y] + 2 * projectileData[x][E_PROJECTILE_MASS] * cy) / (projectileData[i][E_PROJECTILE_MASS] + projectileData[x][E_PROJECTILE_MASS]);
                        cz = ((projectileData[i][E_PROJECTILE_MASS] - projectileData[x][E_PROJECTILE_MASS]) * projectileData[i][E_PROJECTILE_VEL_Z] + 2 * projectileData[x][E_PROJECTILE_MASS] * cz) / (projectileData[i][E_PROJECTILE_MASS] + projectileData[x][E_PROJECTILE_MASS]);
                        
                        new_vx += cx;
                        new_vy += cy;
                        new_vz += cz;

                        projectileData[i][E_PROJECTILE_VEL_X] = new_vx;
                        projectileData[i][E_PROJECTILE_VEL_Y] = new_vy;
                        projectileData[i][E_PROJECTILE_VEL_Z] = new_vz;

                        // OTHER SIMULATION IN LOOP

                        Internal_VonU(projectileData[x][E_PROJECTILE_VEL_X], projectileData[x][E_PROJECTILE_VEL_Y], projectileData[x][E_PROJECTILE_VEL_Z], dx, dy, dz, cx, cy, cz);
                        
                        new_vx -= cx;
                        new_vy -= cy;
                        new_vz -= cz;

                        Internal_VonU(projectileData[i][E_PROJECTILE_VEL_X], projectileData[i][E_PROJECTILE_VEL_Y], projectileData[i][E_PROJECTILE_VEL_Z], -dx, -dy, -dz, cx, cy, cz);
                        
                        cx = ((projectileData[x][E_PROJECTILE_MASS] - projectileData[i][E_PROJECTILE_MASS]) * projectileData[x][E_PROJECTILE_VEL_X] + 2 * projectileData[i][E_PROJECTILE_MASS] * cx) / (projectileData[x][E_PROJECTILE_MASS] + projectileData[i][E_PROJECTILE_MASS]);
                        cy = ((projectileData[x][E_PROJECTILE_MASS] - projectileData[i][E_PROJECTILE_MASS]) * projectileData[x][E_PROJECTILE_VEL_Y] + 2 * projectileData[i][E_PROJECTILE_MASS] * cy) / (projectileData[x][E_PROJECTILE_MASS] + projectileData[i][E_PROJECTILE_MASS]);
                        cz = ((projectileData[x][E_PROJECTILE_MASS] - projectileData[i][E_PROJECTILE_MASS]) * projectileData[x][E_PROJECTILE_VEL_Z] + 2 * projectileData[i][E_PROJECTILE_MASS] * cz) / (projectileData[x][E_PROJECTILE_MASS] + projectileData[i][E_PROJECTILE_MASS]);
                        
                        new_vx += cx;
                        new_vy += cy;
                        new_vz += cz;

                        projectileData[x][E_PROJECTILE_VEL_X] = new_vx;
                        projectileData[x][E_PROJECTILE_VEL_Y] = new_vy;
                        projectileData[x][E_PROJECTILE_VEL_Z] = new_vz;

                        collision = true;

                        CallLocalFunction("OnProjectileCollide", "iiiiii", _:i, PROJECTILE_COLLIDE_SIMULATION, dx, dy, dz, _:x);
                    }
                }
            }
        }

        // collision check with objects
        if ((extraid = CA_RayCastLineAngle(projectileData[i][E_PROJECTILE_X], projectileData[i][E_PROJECTILE_Y], projectileData[i][E_PROJECTILE_Z], new_x, new_y, new_z, cx, cy, cz, crx, cry, unused)) != 0) {
            moveangle = atan2(-cry, crx);
            new_vx = ((projectileData[i][E_PROJECTILE_VEL_X] * floatcos(moveangle, degrees)) - (projectileData[i][E_PROJECTILE_VEL_Y] * floatsin(moveangle, degrees)));
            new_vy = -((projectileData[i][E_PROJECTILE_VEL_X] * floatsin(moveangle, degrees)) + (projectileData[i][E_PROJECTILE_VEL_Y] * floatcos(moveangle, degrees)));

            moveangle *= -1;
            projectileData[i][E_PROJECTILE_VEL_X] = ((new_vx * floatcos(moveangle, degrees)) - (new_vy * floatsin(moveangle, degrees)));
            projectileData[i][E_PROJECTILE_VEL_Y] = ((new_vx * floatsin(moveangle, degrees)) + (new_vy * floatcos(moveangle, degrees)));

            moveangle += ((new_vy > 0) ? (90.0) : (-90.0));
            new_x = (cx + (projectileData[i][E_PROJECTILE_OBJECT_COL_RADIUS] * floatcos(moveangle, degrees)));
            new_y = (cy + (projectileData[i][E_PROJECTILE_OBJECT_COL_RADIUS] * floatsin(moveangle, degrees)));

            collision = true;

            CallLocalFunction("OnProjectileCollide", "iiiiii", _:i, PROJECTILE_COLLIDE_OBJECT, cx, cy, cz, extraid);
        }

        // collision check with players
        if (projectileData[i][E_PROJECTILE_PLAYER_COL_RADIUS] != 0.0) {
            foreach (new p : Player) {
                GetPlayerPos(p, cx, cy, cz);

                if ((cz - (min_height - projectileData[i][E_PROJECTILE_OBJECT_COL_RADIUS])) < new_z < (cz + (max_height + projectileData[i][E_PROJECTILE_OBJECT_COL_RADIUS]))) {
                    dx = new_x - cx;
                    dy = new_y - cy;

                    if (((dx * dx) + (dy * dy)) < ((projectileData[i][E_PROJECTILE_OBJECT_COL_RADIUS] + projectileData[i][E_PROJECTILE_PLAYER_COL_RADIUS]) * (projectileData[i][E_PROJECTILE_OBJECT_COL_RADIUS] + projectileData[i][E_PROJECTILE_PLAYER_COL_RADIUS]))) {
                        if (((projectileData[i][E_PROJECTILE_VEL_X] * dx) + (projectileData[i][E_PROJECTILE_VEL_Y] * dy)) < 0.0) {
                              moveangle = -atan2(dy, dx);
                            new_vx = ((projectileData[i][E_PROJECTILE_VEL_X] * floatcos(moveangle, degrees)) - (projectileData[i][E_PROJECTILE_VEL_Y] * floatsin(moveangle, degrees)));
                            new_vy = ((projectileData[i][E_PROJECTILE_VEL_X] * floatsin(moveangle, degrees)) + (projectileData[i][E_PROJECTILE_VEL_Y] * floatcos(moveangle, degrees)));

                            moveangle *= -1;
                            projectileData[i][E_PROJECTILE_VEL_X] = ((new_vx * floatcos(moveangle, degrees)) - (new_vy * floatsin(moveangle, degrees)));
                            projectileData[i][E_PROJECTILE_VEL_Y] = ((new_vx * floatsin(moveangle, degrees)) + (new_vy * floatcos(moveangle, degrees)));

                            collision = true;

                            CallLocalFunction("OnProjectileCollide", "iiiiii", _:i, PROJECTILE_COLLIDE_PLAYER, cx, cy, cz, p);

                            break;
                        }
                    }
                }
            }
        }

        // apply collision friction
        moveangle = (atan2(projectileData[i][E_PROJECTILE_VEL_Y], projectileData[i][E_PROJECTILE_VEL_X]) - 90.0);
        speed = floatsqroot((projectileData[i][E_PROJECTILE_VEL_X] * projectileData[i][E_PROJECTILE_VEL_X]) + (projectileData[i][E_PROJECTILE_VEL_Y] * projectileData[i][E_PROJECTILE_VEL_Y]));

        if (projectileData[i][E_PROJECTILE_COLLISION_FRICTION] != 0.0 && speed > 0.0 && collision) {
            speed -= projectileData[i][E_PROJECTILE_COLLISION_FRICTION];
            if (speed < 0.001) {
                speed = 0;
            }

            projectileData[i][E_PROJECTILE_VEL_X] = speed * floatsin(-moveangle, degrees);
            projectileData[i][E_PROJECTILE_VEL_Y] = speed * floatcos(-moveangle, degrees);
        }

        // apply ground friction
        if (projectileData[i][E_PROJECTILE_GROUND_FRICTION] != 0.0 && speed > 0.0 && new_z == min_height) {
            speed -= projectileData[i][E_PROJECTILE_GROUND_FRICTION] * (PROJECTILE_TIMER_INTERVAL / 1000.0);
            if (speed < 0.001) {
                speed = 0;
            }

            projectileData[i][E_PROJECTILE_VEL_X] = speed * floatsin(-moveangle, degrees);
            projectileData[i][E_PROJECTILE_VEL_Y] = speed * floatcos(-moveangle, degrees);
        }

        // apply air resistance
        if (projectileData[i][E_PROJECTILE_AIR_RESISTANCE] != 0.0) {
            if ((new_z == min_height && floatabs(projectileData[i][E_PROJECTILE_AIR_RESISTANCE]) > projectileData[i][E_PROJECTILE_GROUND_FRICTION]) || (collision && floatabs(projectileData[i][E_PROJECTILE_AIR_RESISTANCE]) > projectileData[i][E_PROJECTILE_COLLISION_FRICTION]) || new_z > min_height) {
                projectileData[i][E_PROJECTILE_VEL_X] -= projectileData[i][E_PROJECTILE_VEL_X] * projectileData[i][E_PROJECTILE_AIR_RESISTANCE] * (PROJECTILE_TIMER_INTERVAL / 1000.0);
                projectileData[i][E_PROJECTILE_VEL_Y] -= projectileData[i][E_PROJECTILE_VEL_Y] * projectileData[i][E_PROJECTILE_AIR_RESISTANCE] * (PROJECTILE_TIMER_INTERVAL / 1000.0);
            }
        }

        // update rotation
        speed = floatsqroot((projectileData[i][E_PROJECTILE_VEL_X] * projectileData[i][E_PROJECTILE_VEL_X]) + (projectileData[i][E_PROJECTILE_VEL_Y] * projectileData[i][E_PROJECTILE_VEL_Y]));
        if (speed > 0.0) {
            projectileData[i][E_PROJECTILE_ROT_X] -= ((speed * (PROJECTILE_TIMER_INTERVAL / 1000.0)) * ((180.0 / 3.14159) / projectileData[i][E_PROJECTILE_OBJECT_COL_RADIUS]));
            if (projectileData[i][E_PROJECTILE_ROT_X] < 0.0) {
                projectileData[i][E_PROJECTILE_ROT_X] += 360.0;
            }

            projectileData[i][E_PROJECTILE_ROT_Z] = moveangle;
        }

        // update position
        projectileData[i][E_PROJECTILE_X] = new_x;
        projectileData[i][E_PROJECTILE_Y] = new_y;
        projectileData[i][E_PROJECTILE_Z] = new_z;

        CallLocalFunction("OnProjectileUpdate", "i", _:i);

        // if velocity is 0, stop simulation (KillTimer)
        if (projectileData[i][E_PROJECTILE_VEL_X] == 0.0 && projectileData[i][E_PROJECTILE_VEL_Y] == 0.0 && ((new_z == min_height && projectileData[i][E_PROJECTILE_VEL_Z] <= 0.0) || (new_z == max_height && projectileData[i][E_PROJECTILE_VEL_Z] >= 0.0))) {
            CallLocalFunction("OnProjectileStop", "iiii", _:i, projectileData[i][E_PROJECTILE_X], projectileData[i][E_PROJECTILE_Y], projectileData[i][E_PROJECTILE_Z]);
            DestroyProjectile(i);
        }
    }

    return 1;
}
