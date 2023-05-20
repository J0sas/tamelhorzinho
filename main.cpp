#include "memory.hpp"
#include "offsets.hpp"
#include <cmath>

struct glow_t
{
    float r, g, b, a;
};


//  basics
struct Vector
{
    // coordinates
    float x{};
    float y{};
    float z{};
};


// calc angle have to do three things, first get a source vector with the values, a destination vector which will be saved
// and the angles vector
float CalcMagnitude(const Vector& coordinate_calculations) 
{
    // calculate the hipotenuze
    float magnitude = sqrt(coordinate_calculations.x * coordinate_calculations.x + coordinate_calculations.y * coordinate_calculations.y + coordinate_calculations.z * coordinate_calculations.z);
    return magnitude;
}

void subtract_vectors(const Vector& vector1, const Vector& vector2, Vector& resultant_pointer)
{
    resultant_pointer.x = vector1.x - vector2.x;
    resultant_pointer.y = vector1.y - vector2.y;
    resultant_pointer.z = vector1.z - vector2.z;
    
}

//  basics

int main()
{
    // testing my CalcVec function 
    Vector vec1{ 2.f, 3.f, 0.f };
    Vector vec2{ -2.f, 1.f, 0.f };
    Vector Result{};
    subtract_vectors(vec1, vec2, Result);
    std::cout << "Result is: (" << Result.x << "," << Result.y << "," << Result.z << ")" << '\n';
    
    std::cin.get();

    // Result: Success, it is working :D

    
    // get csgo handle
    const auto mem = memory(L"csgo.exe");
    
    // get client.dll address
    const auto client = mem.get_module_address(L"client.dll");
    if (client == 0x0) {
        std::cout << "could not get client.dll";
    }
    else
    {
        std::cout << "client address: " << std::hex << std::uppercase << client << '\n';
    }

    while (true) {
        // sleep so our pc does not die
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // get local player, mflags and crosshair(this crosshair will be used later)

        const auto local_player = mem.read<std::uintptr_t>(client + offsets::signatures::dwLocalPlayer);
        const auto local_player_flags = mem.read<std::int32_t>(local_player + offsets::netvars::m_fFlags);
        const auto local_player_team = mem.read<std::uintptr_t>(local_player + offsets::netvars::m_iTeamNum);
        
        const auto glow_object_manager = mem.read<std::uintptr_t>(client + offsets::signatures::dwGlowObjectManager);
        
        const auto crosshair_id = mem.read<std::int16_t>(local_player + offsets::netvars::m_iCrosshairId);
        
        const auto entity_for_triggerbot = mem.read<std::uintptr_t>(client + offsets::signatures::dwEntityList + ((crosshair_id - 1) * 0x10));
        
        const glow_t color{ 0.89019f, 0.23137f, 0.23137f, 0.8f };
        
        //aimbot

        for (std::int32_t i = 1; i < 64; i++)
        {

        }


        
        //triggerbot
        //todo: find a way to deal with recoil
        if (GetAsyncKeyState(VK_XBUTTON1))
        {
            if (crosshair_id > 0 && crosshair_id < 64)
            {

                // check if the player is on my team
                if ((mem.read<std::uintptr_t>(entity_for_triggerbot + offsets::netvars::m_iTeamNum) == local_player_team)) {
                    continue;
                }
                // shoots :D 
                
                mem.write<std::uintptr_t>(client + offsets::signatures::dwForceAttack, 6); // +attack
            }
        }
        //glow
        for (auto i = 0; i < 64; i++)
        {
            const auto entity = mem.read<std::uintptr_t>(client + offsets::signatures::dwEntityList + (i * 0x10)); 
            // get the location of each entity, from 0 to 64.

            if (!entity)
                continue;

            // check if they are on my team

            if (mem.read<std::uintptr_t>(entity + offsets::netvars::m_iTeamNum) == local_player_team)
                continue;


            const auto glow_index = mem.read<std::int32_t>(entity + offsets::netvars::m_iGlowIndex);

           /* std::cout << "entity: " << entity << '\n';
            std::cout << "glow_index: " << glow_index << '\n';*/

            mem.write<glow_t>(glow_object_manager + (glow_index * 0x38) + 0x8, color); 
            // write our glow structure, so we dont have to make multiples WPM

            mem.write<bool>(glow_object_manager + (glow_index * 0x38) + 0x28, true); // render when occluded
            mem.write<bool>(glow_object_manager + (glow_index * 0x38) + 0x29, false); // render when unoccluded
        }
        
        // bhop
        if (GetAsyncKeyState(VK_SPACE))
        {
            if (local_player_flags & (1 << 0))
            {
                std::cout << local_player_flags << '\n';
                mem.write<std::int32_t>(client + offsets::signatures::dwForceJump, 5);
            }
            else
            {
                if(mem.read<std::int32_t>(client + offsets::signatures::dwForceJump) == 5)
                    mem.write<std::int32_t>(client + offsets::signatures::dwForceJump, 4);

            }
        }
       
                
        if (GetAsyncKeyState(VK_DELETE))
        {
            return 0;
        }
    }

    return 0;
}

// refatorar o código e deixar modular
// criar um método só para retornar os endereços do client.dll e calcular os offsets