// target/riscv/insn_trans/trans_cx.c.inc

static bool gen_cx_reg_r(DisasContext *ctx, arg_cx_reg *a,
                          void (*func)(TCGv, TCGv_ptr, TCGv, TCGv, TCGv))
{
    TCGv dest = dest_gpr(ctx, a->rd);
    TCGv cf_id = tcg_constant_tl(a->cf_id);
    TCGv src1 = get_gpr(ctx, a->rs1, EXT_NONE);
    TCGv src2 = get_gpr(ctx, a->rs2, EXT_NONE);   

    func(dest, tcg_env, cf_id, src1, src2);
    gen_set_gpr(ctx, a->rd, dest);
    return true;
}

static bool trans_cx_reg(DisasContext *ctx, arg_cx_reg *a) 
{
    return gen_cx_reg_r(ctx, a, gen_helper_cx_reg);
}