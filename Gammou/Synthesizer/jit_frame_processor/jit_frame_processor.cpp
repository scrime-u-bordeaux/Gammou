
#include "jit_frame_processor.h"

#include <cstring>
#include <cstdint>


#ifdef __linux__
#include <sys/mman.h>
#elif defined(_WIN32)
#include <WinBase.h>
#else
#error "jit_compiler not implemented for this os"
#endif


#define PROGRAM_SIZE 8192

namespace Gammou {

    namespace Sound {

        jit_frame_processor::jit_frame_processor() 
         : m_memory(42)
        {
			m_program = (uint8_t*)alloc_executable(PROGRAM_SIZE);
            clear_program();
        }

        jit_frame_processor::~jit_frame_processor()
        {
            free_executable(m_program, PROGRAM_SIZE);
        }

        void jit_frame_processor::execute_process_program()
        {
            void (*fct)() = (void (*)())m_program;
            fct();
        }

        void jit_frame_processor::execute_initialize_program()
        {
            // TODO
        }

        void jit_frame_processor::compile_component(Process::abstract_component<double>* component)
        {
			const unsigned int current_process_cycle = get_process_cycle();

            // A ret Instruction is always at the end of the program, lets delete it
            m_program_size--;

            // Generate code for the component
            compile_component_aux(component, current_process_cycle);

            // Reput ret instruction at the end of the new program
            add_ret();

            DEBUG_PRINT("JIT Compililation : code size = %lu\n", m_program_size);
        }

        void jit_frame_processor::compile_component_aux(
            Process::abstract_component<double>* component,
            const unsigned int process_cycle)
        {
            if (component->update_process_cyle(process_cycle)) {
                const unsigned int ic = component->get_input_count();

                // TODO Initialization list

                if (ic != 0) {
					Process::abstract_component<double> *src_component[16];
                    unsigned int src_output_id[16];

                    // Compile each component pluged on input

                    for (unsigned int i = 0; i < ic; ++i) {
						src_component[i] = component->get_input_src(i, src_output_id[i]);

                        if (src_component[i] != nullptr)
                            compile_component_aux(src_component[i], process_cycle);
                    }
					
                    // Fetch components output values

                    for (unsigned int i = 0; i < ic; ++i) {
                        if (src_component[i] != nullptr)
							add_fetch_output(src_component[i], src_output_id[i], &(m_memory[i]));
                        else
							add_fetch_default(&(m_memory[i]));
                    }

                }

                // 
                add_process(component, m_memory.data());
            }
        }

        void jit_frame_processor::clear_program()
        {
            m_program_size = 0;
            // Add return Instruction
            add_ret();
        }

        /*
            Executable memory management
        */

        void *jit_frame_processor::alloc_executable(const size_t size)
        {
			void *ret = nullptr;
#ifdef __linux__
			ret = mmap(0, size, 
						PROT_READ | PROT_WRITE | PROT_EXEC,
						MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

			if (ret == (void*)-1)
				throw std::runtime_error("mmap failed");
#elif defined(_WIN32)
			ret = VirtualAlloc(
				nullptr, size, 
				MEM_COMMIT | MEM_RESERVE, 
				PAGE_EXECUTE_READWRITE);

			if (ret == nullptr)
				throw std::runtime_error("Virtual Alloc Failed");
#endif
            return ret;
        }

        void jit_frame_processor::free_executable(void *mem, const size_t size)
        {
#ifdef __linux__
			munmap(mem, size);
#elif defined(_WIN32)
			VirtualFree(mem, size, MEM_RELEASE);
#endif
        }

        /*

        */

        void jit_frame_processor::__fetch_output(
            Process::abstract_component<double> *component,
            const unsigned int output_id,
            double *dest)
        {
            *dest = component->fetch_output(output_id);
        }

        void jit_frame_processor::__process(
            Process::abstract_component<double> *component,
            const double *input)
        {
            component->process(input);
        }

        /*
            Instruction Writing
        */

       void jit_frame_processor::add_program_chunk(const void *chunk, const size_t size)
       {
           std::memcpy(m_program + m_program_size, chunk, size);
           m_program_size += size;
       }

       void jit_frame_processor::add_mov_ptr_rax(void *ptr)
       {
           const uint16_t code_chunk = 0xb848;
            add_program_chunk(&code_chunk, 2);
            add_program_chunk(&ptr, 8);
       }

        void jit_frame_processor::add_mov_ptr_rdi(void *ptr)
        {
            const uint16_t code_chunk = 0xbf48;
            add_program_chunk(&code_chunk, 2);
            add_program_chunk(&ptr, 8);
        }

        void jit_frame_processor::add_mov_ptr_rsi(void *ptr)
        {
            const uint16_t code_chunk = 0xbe48;
            add_program_chunk(&code_chunk, 2);
            add_program_chunk(&ptr, 8);
        }

        void jit_frame_processor::add_mov_ptr_rdx(void *ptr)
        {
            const uint16_t code_chunk = 0xba48;
            add_program_chunk(&code_chunk, 2);
            add_program_chunk(&ptr, 8);
        }

		void jit_frame_processor::add_mov_ptr_rcx(void * ptr)
		{
			const uint16_t code_chunk = 0xb948;
			add_program_chunk(&code_chunk, 2);
			add_program_chunk(&ptr, 8);
		}

		void jit_frame_processor::add_mov_ptr_r8(void * ptr)
		{
			const uint16_t code_chunk = 0xb849;
			add_program_chunk(&code_chunk, 2);
			add_program_chunk(&ptr, 8);
		}

        void jit_frame_processor::add_mov_int_esi(const uint32_t value)
        {
            const uint8_t code_chunk = 0xbe;
            add_program_chunk(&code_chunk, 1);
            add_program_chunk(&value, 4);
        }

		void jit_frame_processor::add_mov_int_rdx(const uint32_t value)
		{
			const uint64_t tmp = value;
			const uint16_t code_chunk = 0xba48;
			add_program_chunk(&code_chunk, 2);
			add_program_chunk(&tmp, 8);
		}

        void jit_frame_processor::add_ret()
        {
            const uint8_t code_chunk = 0xc3;
            add_program_chunk(&code_chunk, 1);
        }

        void jit_frame_processor::add_fetch_output(
            Process::abstract_component<double>* component, 
            const unsigned int output_id, 
            double *mem_pos)
        {
#ifdef __linux__
			const uint8_t code_chunk[] = 
            {
                0xff, 0xd0      //  callq *%rax
            };

            add_mov_ptr_rdi(component);
            add_mov_int_esi(output_id);
            add_mov_ptr_rdx(mem_pos);
            add_mov_ptr_rax(reinterpret_cast<void*>(__fetch_output));

#elif defined(_WIN32)
#error "not implemented"
#endif
            add_program_chunk(code_chunk, sizeof(code_chunk));
        }

        void jit_frame_processor::add_fetch_default(double *mem_pos)
        {
#ifdef __linux__
            const uint8_t code_chunk[] = 
            {
                0x48, 0xc7, 0x07, 0x00,		//	movq $0, (%rdi)
                0x00, 0x00, 0x00
            };

            add_mov_ptr_rdi(mem_pos);
#elif defined(_WIN32)
#error "not implemented"
#endif
            add_program_chunk(code_chunk, sizeof(code_chunk));
        }

        void jit_frame_processor::add_process(Process::abstract_component<double>* component, double *input)
        {
#ifdef __linux__
            const uint8_t code_chunk[] =
            {
                0xff, 0xd0 //  callq *%rax
            };

            add_mov_ptr_rdi(component);
            add_mov_ptr_rsi(input);
            add_mov_ptr_rax(reinterpret_cast<void*>(__process));

#elif defined(_WIN32)
#error "not implemented"
#endif
            add_program_chunk(code_chunk, sizeof(code_chunk));
        }



    } /* Sound */

} /* Gammou */ 


