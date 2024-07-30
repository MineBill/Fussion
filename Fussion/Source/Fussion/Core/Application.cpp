#include "e5pch.h"
#include "Application.h"

#include "Time.h"

#include <tracy/Tracy.hpp>

#include "Fussion/Input/Input.h"
#include "Fussion/OS/Clock.h"
#include "Fussion/RHI/Renderer.h"
#include "Fussion/Scene/Components/BaseComponents.h"
#include "Scripting/ScriptingEngine.h"

#include <tracy/TracyC.h>

namespace Fussion {
Application* Application::s_Instance = nullptr;

class SimpleSink final : public LogSink {
    Application* m_Application;

public:
    explicit SimpleSink(Application* app): m_Application(app) {}

    virtual void Write(LogLevel level, std::string_view message, std::source_location const& loc) override
    {
        m_Application->OnLogReceived(level, message, loc);
    }
};

void Application::Run()
{
    s_Instance = this;
    Log::DefaultLogger()->SetLogLevel(LogLevel::Debug);
    Log::DefaultLogger()->RegisterSink(MakeRef<SimpleSink>(this));

    WindowOptions options{
        .InitialTitle = "Window",
        .InitialWidth = 1366,
        .InitialHeight = 768,
        .Flags = WindowFlag::Centered,
    };
    m_Window.reset(Window::Create(options));
    m_Window->OnEvent([this](Event& event) -> bool {
        ZoneScoped;
        OnEvent(event);
        Input::OnEvent(event);
        for (auto const& layer : m_Layers) {
            if (event.Handled) {
                break;
            }
            layer->OnEvent(event);
        }
        return false;
    });

    ScriptingEngine::Initialize();
    defer(ScriptingEngine::Shutdown());

    RHI::Renderer::Init(*m_Window.get());
    RHI::Renderer::GetInstance()->CreateDefaultRenderpasses();

    OnStart();

    RHI::Renderer::GetInstance()->CreateDefaultResources();

    Clock clock;
    while (!m_Quit) {
        auto const delta = CAST(f32, clock.Reset()) / 1000.0f;
        Time::SetDeltaTime(delta);
        m_Window->Update();
        OnUpdate(delta);

        Input::Flush();
        FrameMark;
    }
}

void Application::PushLayer(Layer* layer)
{
    m_Layers.push_back(layer);
}

void Application::Quit()
{
    LOG_DEBUG("Quit was requested");
    m_Quit = true;
}
}

// #include <iostream>
// #include <cstddef>
// #include <cassert>
// #if defined(_WIN32) || defined(_WIN64)
// #include <windows.h>
// #undef max
// #else
// #include <sys/mman.h>
// #include <unistd.h>
// #endif
//
// class ArenaAllocator {
// public:
//     explicit ArenaAllocator(size_t initialSize)
//         : _initialSize(initialSize), _currentBlock(nullptr), _currentBlockSize(0), _currentOffset(0), _blockList(nullptr) {
//         AllocateNewBlock(initialSize);
//     }
//
//     ~ArenaAllocator() {
//         FreeAllBlocks();
//     }
//
//     void* Allocate(size_t size) {
//         size = (size + 7) & ~7; // Align size to 8 bytes
//
//         if (_currentOffset + size > _currentBlockSize) {
//             size_t newBlockSize = std::max(_initialSize, size);
//             AllocateNewBlock(newBlockSize);
//         }
//
//         void* ptr = static_cast<char*>(_currentBlock) + _currentOffset;
//         _currentOffset += size;
//         return ptr;
//     }
//
//     void Reset() {
//         FreeAllBlocks();
//         AllocateNewBlock(_initialSize);
//     }
//
//     size_t Used() const {
//         size_t used = 0;
//         for (Block* block = _blockList; block != nullptr; block = block->next) {
//             used += block->size;
//         }
//         used -= (_currentBlockSize - _currentOffset);
//         return used;
//     }
//
//     size_t Capacity() const {
//         size_t capacity = 0;
//         for (Block* block = _blockList; block != nullptr; block = block->next) {
//             capacity += block->size;
//         }
//         return capacity;
//     }
//
// private:
//     struct Block {
//         Block* next;
//         size_t size;
//         void* memory;
//     };
//
//     void AllocateNewBlock(size_t size) {
//         void* newMemory = nullptr;
//
//         #if defined(_WIN32) || defined(_WIN64)
//             newMemory = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
//         #else
//             newMemory = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
//             if (newMemory == MAP_FAILED) {
//                 newMemory = nullptr;
//             }
//         #endif
//
//         if (!newMemory) {
//             throw std::bad_alloc();
//         }
//
//         Block* newBlock = reinterpret_cast<Block*>(AllocateBlockStruct());
//         newBlock->memory = newMemory;
//         newBlock->size = size;
//         newBlock->next = _blockList;
//         _blockList = newBlock;
//
//         _currentBlock = newMemory;
//         _currentBlockSize = size;
//         _currentOffset = 0;
//     }
//
//     void* AllocateBlockStruct() {
//         // Allocate space for a Block structure in the currently active block.
//         size_t blockStructSize = sizeof(Block);
//         blockStructSize = (blockStructSize + 7) & ~7; // Align size to 8 bytes
//
//         if (_currentOffset + blockStructSize > _currentBlockSize) {
//             // Allocate separately if there's not enough space in the current block.
//             size_t pageSize = 4096; // Assume a common page size
//             #if defined(_WIN32) || defined(_WIN64)
//                 return VirtualAlloc(nullptr, pageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
//             #else
//                 void* ptr = mmap(nullptr, pageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
//                 if (ptr == MAP_FAILED) {
//                     throw std::bad_alloc();
//                 }
//                 return ptr;
//             #endif
//         }
//
//         void* blockStructPtr = static_cast<char*>(_currentBlock) + _currentOffset;
//         _currentOffset += blockStructSize;
//         return blockStructPtr;
//     }
//
//     void FreeAllBlocks() {
//         Block* block = _blockList;
//         while (block) {
//             Block* next = block->next;
//             #if defined(_WIN32) || defined(_WIN64)
//                 VirtualFree(block->memory, 0, MEM_RELEASE);
//                 VirtualFree(block, 0, MEM_RELEASE); // Free block struct if separately allocated
//             #else
//                 munmap(block->memory, block->size);
//                 munmap(block, sizeof(Block));
//             #endif
//             block = next;
//         }
//         _blockList = nullptr;
//         _currentBlock = nullptr;
//         _currentBlockSize = 0;
//         _currentOffset = 0;
//     }
//
//     size_t _initialSize;     // Initial size of the blocks
//     void* _currentBlock;     // Pointer to the current block
//     size_t _currentBlockSize; // Size of the current block
//     size_t _currentOffset;   // Current offset in the current block
//
//     Block* _blockList;       // Linked list of allocated blocks
// };
//
// static ArenaAllocator s_Allocator{10'000'000};

void* operator new(std::size_t size)
{
    auto ptr = malloc(size);
    TracyAllocS(ptr, size, 15);
    return ptr;
}

void operator delete(void* ptr) noexcept
{
    TracyFreeS(ptr, 15);
    free(ptr);
}
