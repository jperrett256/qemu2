/*
 * ARM gdb server stub: AArch64 specific functions.
 *
 * Copyright (c) 2013 SUSE LINUX Products GmbH
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#include "qemu/osdep.h"
#include "cpu.h"
#include "exec/gdbstub.h"

int aarch64_cpu_gdb_read_register(CPUState *cs, GByteArray *mem_buf, int n)
{
    ARMCPU *cpu = ARM_CPU(cs);
    CPUARMState *env = &cpu->env;

    if (n < 31) {
        /* Core integer register.  */
        return gdb_get_reg64(mem_buf, arm_get_xreg(env, n));
    }
    switch (n) {
    case 31:
        return gdb_get_reg64(mem_buf, arm_get_xreg(env, n));
    case 32:
        return gdb_get_reg64(mem_buf, get_aarch_reg_as_x(&env->pc));
    case 33:
        return gdb_get_reg32(mem_buf, pstate_read(env));
    }
    /* Unknown register.  */
    return 0;
}

int aarch64_cpu_gdb_write_register(CPUState *cs, uint8_t *mem_buf, int n)
{
    ARMCPU *cpu = ARM_CPU(cs);
    CPUARMState *env = &cpu->env;
    uint64_t tmp;

    tmp = ldq_p(mem_buf);

    if (n < 31) {
        /* Core integer register.  */
        arm_set_xreg(env, n, tmp);
        return 8;
    }
    switch (n) {
    case 31:
        arm_set_xreg(env, n, tmp);
        return 8;
    case 32:
        set_aarch_reg_to_x(&env->pc, tmp);
        return 8;
    case 33:
        /* CPSR */
        pstate_write(env, tmp);
        return 4;
    }
    /* Unknown register.  */
    return 0;
}
