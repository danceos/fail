#include "udis86_helper.hpp"

#include <iostream>

char
Udis86Helper::operandTypeToChar(unsigned op)
{
	switch (op) {
	case UD_NONE: return '-';
	case UD_OP_MEM: return 'M';
	case UD_OP_PTR: return 'P';
	case UD_OP_IMM: return 'I';
	case UD_OP_JIMM: return 'J';
	case UD_OP_CONST: return 'C';
	case UD_OP_REG: return 'R';
	default: return '?';
	}
}

unsigned
Udis86Helper::operandCount()
{
	return (ud->operand[0].type != UD_NONE) +
	       (ud->operand[1].type != UD_NONE) +
	       (ud->operand[2].type != UD_NONE);
}

#define CASE(x) case x: return #x;
char const *
Udis86Helper::mnemonicToString(unsigned mnemonic)
{

	switch (mnemonic) {
		CASE(UD_Iinvalid) CASE(UD_I3dnow) CASE(UD_Inone) CASE(UD_Idb) CASE(UD_Ipause) CASE(UD_Iaaa)
		CASE(UD_Iaad) CASE(UD_Iaam) CASE(UD_Iaas) CASE(UD_Iadc) CASE(UD_Iadd) CASE(UD_Iaddpd)
		CASE(UD_Iaddps) CASE(UD_Iaddsd) CASE(UD_Iaddss) CASE(UD_Iand) CASE(UD_Iandpd) CASE(UD_Iandps)
		CASE(UD_Iandnpd) CASE(UD_Iandnps) CASE(UD_Iarpl) CASE(UD_Imovsxd) CASE(UD_Ibound) CASE(UD_Ibsf)
		CASE(UD_Ibsr) CASE(UD_Ibswap) CASE(UD_Ibt) CASE(UD_Ibtc) CASE(UD_Ibtr) CASE(UD_Ibts)
		CASE(UD_Icall) CASE(UD_Icbw) CASE(UD_Icwde) CASE(UD_Icdqe) CASE(UD_Iclc) CASE(UD_Icld)
		CASE(UD_Iclflush) CASE(UD_Iclgi) CASE(UD_Icli) CASE(UD_Iclts) CASE(UD_Icmc) CASE(UD_Icmovo)
		CASE(UD_Icmovno) CASE(UD_Icmovb) CASE(UD_Icmovae) CASE(UD_Icmovz) CASE(UD_Icmovnz) CASE(UD_Icmovbe)
		CASE(UD_Icmova) CASE(UD_Icmovs) CASE(UD_Icmovns) CASE(UD_Icmovp) CASE(UD_Icmovnp) CASE(UD_Icmovl)
		CASE(UD_Icmovge) CASE(UD_Icmovle) CASE(UD_Icmovg) CASE(UD_Icmp) CASE(UD_Icmppd) CASE(UD_Icmpps)
		CASE(UD_Icmpsb) CASE(UD_Icmpsw) CASE(UD_Icmpsd) CASE(UD_Icmpsq) CASE(UD_Icmpss) CASE(UD_Icmpxchg)
		CASE(UD_Icmpxchg8b) CASE(UD_Icomisd) CASE(UD_Icomiss) CASE(UD_Icpuid) CASE(UD_Icvtdq2pd) CASE(UD_Icvtdq2ps)
		CASE(UD_Icvtpd2dq) CASE(UD_Icvtpd2pi) CASE(UD_Icvtpd2ps) CASE(UD_Icvtpi2ps) CASE(UD_Icvtpi2pd) CASE(UD_Icvtps2dq)
		CASE(UD_Icvtps2pi) CASE(UD_Icvtps2pd) CASE(UD_Icvtsd2si) CASE(UD_Icvtsd2ss) CASE(UD_Icvtsi2ss) CASE(UD_Icvtss2si)
		CASE(UD_Icvtss2sd) CASE(UD_Icvttpd2pi) CASE(UD_Icvttpd2dq) CASE(UD_Icvttps2dq) CASE(UD_Icvttps2pi) CASE(UD_Icvttsd2si)
		CASE(UD_Icvtsi2sd) CASE(UD_Icvttss2si) CASE(UD_Icwd) CASE(UD_Icdq) CASE(UD_Icqo) CASE(UD_Idaa)
		CASE(UD_Idas) CASE(UD_Idec) CASE(UD_Idiv) CASE(UD_Idivpd) CASE(UD_Idivps) CASE(UD_Idivsd)
		CASE(UD_Idivss) CASE(UD_Iemms) CASE(UD_Ienter) CASE(UD_If2xm1) CASE(UD_Ifabs) CASE(UD_Ifadd)
		CASE(UD_Ifaddp) CASE(UD_Ifbld) CASE(UD_Ifbstp) CASE(UD_Ifchs) CASE(UD_Ifclex) CASE(UD_Ifcmovb)
		CASE(UD_Ifcmove) CASE(UD_Ifcmovbe) CASE(UD_Ifcmovu) CASE(UD_Ifcmovnb) CASE(UD_Ifcmovne) CASE(UD_Ifcmovnbe)
		CASE(UD_Ifcmovnu) CASE(UD_Ifucomi) CASE(UD_Ifcom) CASE(UD_Ifcom2) CASE(UD_Ifcomp3) CASE(UD_Ifcomi)
		CASE(UD_Ifucomip) CASE(UD_Ifcomip) CASE(UD_Ifcomp) CASE(UD_Ifcomp5) CASE(UD_Ifcompp) CASE(UD_Ifcos)
		CASE(UD_Ifdecstp) CASE(UD_Ifdiv) CASE(UD_Ifdivp) CASE(UD_Ifdivr) CASE(UD_Ifdivrp) CASE(UD_Ifemms)
		CASE(UD_Iffree) CASE(UD_Iffreep) CASE(UD_Ificom) CASE(UD_Ificomp) CASE(UD_Ifild) CASE(UD_Ifncstp)
		CASE(UD_Ifninit) CASE(UD_Ifiadd) CASE(UD_Ifidivr) CASE(UD_Ifidiv) CASE(UD_Ifisub) CASE(UD_Ifisubr)
		CASE(UD_Ifist) CASE(UD_Ifistp) CASE(UD_Ifisttp) CASE(UD_Ifld) CASE(UD_Ifld1) CASE(UD_Ifldl2t)
		CASE(UD_Ifldl2e) CASE(UD_Ifldlpi) CASE(UD_Ifldlg2) CASE(UD_Ifldln2) CASE(UD_Ifldz) CASE(UD_Ifldcw)
		CASE(UD_Ifldenv) CASE(UD_Ifmul) CASE(UD_Ifmulp) CASE(UD_Ifimul) CASE(UD_Ifnop) CASE(UD_Ifpatan)
		CASE(UD_Ifprem) CASE(UD_Ifprem1) CASE(UD_Ifptan) CASE(UD_Ifrndint) CASE(UD_Ifrstor) CASE(UD_Ifnsave)
		CASE(UD_Ifscale) CASE(UD_Ifsin) CASE(UD_Ifsincos) CASE(UD_Ifsqrt) CASE(UD_Ifstp) CASE(UD_Ifstp1)
		CASE(UD_Ifstp8) CASE(UD_Ifstp9) CASE(UD_Ifst) CASE(UD_Ifnstcw) CASE(UD_Ifnstenv) CASE(UD_Ifnstsw)
		CASE(UD_Ifsub) CASE(UD_Ifsubp) CASE(UD_Ifsubr) CASE(UD_Ifsubrp) CASE(UD_Iftst) CASE(UD_Ifucom)
		CASE(UD_Ifucomp) CASE(UD_Ifucompp) CASE(UD_Ifxam) CASE(UD_Ifxch) CASE(UD_Ifxch4) CASE(UD_Ifxch7)
		CASE(UD_Ifxrstor) CASE(UD_Ifxsave) CASE(UD_Ifpxtract) CASE(UD_Ifyl2x) CASE(UD_Ifyl2xp1) CASE(UD_Ihlt)
		CASE(UD_Iidiv) CASE(UD_Iin) CASE(UD_Iimul) CASE(UD_Iinc) CASE(UD_Iinsb) CASE(UD_Iinsw)
		CASE(UD_Iinsd) CASE(UD_Iint1) CASE(UD_Iint3) CASE(UD_Iint) CASE(UD_Iinto) CASE(UD_Iinvd)
		CASE(UD_Iinvept) CASE(UD_Iinvlpg) CASE(UD_Iinvlpga) CASE(UD_Iinvvpid) CASE(UD_Iiretw) CASE(UD_Iiretd)
		CASE(UD_Iiretq) CASE(UD_Ijo) CASE(UD_Ijno) CASE(UD_Ijb) CASE(UD_Ijae) CASE(UD_Ijz)
		CASE(UD_Ijnz) CASE(UD_Ijbe) CASE(UD_Ija) CASE(UD_Ijs) CASE(UD_Ijns) CASE(UD_Ijp)
		CASE(UD_Ijnp) CASE(UD_Ijl) CASE(UD_Ijge) CASE(UD_Ijle) CASE(UD_Ijg) CASE(UD_Ijcxz)
		CASE(UD_Ijecxz) CASE(UD_Ijrcxz) CASE(UD_Ijmp) CASE(UD_Ilahf) CASE(UD_Ilar) CASE(UD_Ilddqu)
		CASE(UD_Ildmxcsr) CASE(UD_Ilds) CASE(UD_Ilea) CASE(UD_Iles) CASE(UD_Ilfs) CASE(UD_Ilgs)
		CASE(UD_Ilidt) CASE(UD_Ilss) CASE(UD_Ileave) CASE(UD_Ilfence) CASE(UD_Ilgdt) CASE(UD_Illdt)
		CASE(UD_Ilmsw) CASE(UD_Ilock) CASE(UD_Ilodsb) CASE(UD_Ilodsw) CASE(UD_Ilodsd) CASE(UD_Ilodsq)
		CASE(UD_Iloopnz) CASE(UD_Iloope) CASE(UD_Iloop) CASE(UD_Ilsl) CASE(UD_Iltr) CASE(UD_Imaskmovq)
		CASE(UD_Imaxpd) CASE(UD_Imaxps) CASE(UD_Imaxsd) CASE(UD_Imaxss) CASE(UD_Imfence) CASE(UD_Iminpd)
		CASE(UD_Iminps) CASE(UD_Iminsd) CASE(UD_Iminss) CASE(UD_Imonitor) CASE(UD_Imontmul) CASE(UD_Imov)
		CASE(UD_Imovapd) CASE(UD_Imovaps) CASE(UD_Imovd) CASE(UD_Imovhpd) CASE(UD_Imovhps) CASE(UD_Imovlhps)
		CASE(UD_Imovlpd) CASE(UD_Imovlps) CASE(UD_Imovhlps) CASE(UD_Imovmskpd) CASE(UD_Imovmskps) CASE(UD_Imovntdq)
		CASE(UD_Imovnti) CASE(UD_Imovntpd) CASE(UD_Imovntps) CASE(UD_Imovntq) CASE(UD_Imovq) CASE(UD_Imovsb)
		CASE(UD_Imovsw) CASE(UD_Imovsd) CASE(UD_Imovsq) CASE(UD_Imovss) CASE(UD_Imovsx) CASE(UD_Imovupd)
		CASE(UD_Imovups) CASE(UD_Imovzx) CASE(UD_Imul) CASE(UD_Imulpd) CASE(UD_Imulps) CASE(UD_Imulsd)
		CASE(UD_Imulss) CASE(UD_Imwait) CASE(UD_Ineg) CASE(UD_Inop) CASE(UD_Inot) CASE(UD_Ior)
		CASE(UD_Iorpd) CASE(UD_Iorps) CASE(UD_Iout) CASE(UD_Ioutsb) CASE(UD_Ioutsw) CASE(UD_Ioutsd)
		CASE(UD_Ioutsq) CASE(UD_Ipacksswb) CASE(UD_Ipackssdw) CASE(UD_Ipackuswb) CASE(UD_Ipaddb) CASE(UD_Ipaddw)
		CASE(UD_Ipaddd) CASE(UD_Ipaddsb) CASE(UD_Ipaddsw) CASE(UD_Ipaddusb) CASE(UD_Ipaddusw) CASE(UD_Ipand)
		CASE(UD_Ipandn) CASE(UD_Ipavgb) CASE(UD_Ipavgw) CASE(UD_Ipcmpeqb) CASE(UD_Ipcmpeqw) CASE(UD_Ipcmpeqd)
		CASE(UD_Ipcmpgtb) CASE(UD_Ipcmpgtw) CASE(UD_Ipcmpgtd) CASE(UD_Ipextrb) CASE(UD_Ipextrd) CASE(UD_Ipextrq)
		CASE(UD_Ipextrw) CASE(UD_Ipinsrw) CASE(UD_Ipmaddwd) CASE(UD_Ipmaxsw) CASE(UD_Ipmaxub) CASE(UD_Ipminsw)
		CASE(UD_Ipminub) CASE(UD_Ipmovmskb) CASE(UD_Ipmulhuw) CASE(UD_Ipmulhw) CASE(UD_Ipmullw) CASE(UD_Ipop)
		CASE(UD_Ipopa) CASE(UD_Ipopad) CASE(UD_Ipopfw) CASE(UD_Ipopfd) CASE(UD_Ipopfq) CASE(UD_Ipor)
		CASE(UD_Iprefetch) CASE(UD_Iprefetchnta) CASE(UD_Iprefetcht0) CASE(UD_Iprefetcht1) CASE(UD_Iprefetcht2) CASE(UD_Ipsadbw)
		CASE(UD_Ipshufw) CASE(UD_Ipsllw) CASE(UD_Ipslld) CASE(UD_Ipsllq) CASE(UD_Ipsraw) CASE(UD_Ipsrad)
		CASE(UD_Ipsrlw) CASE(UD_Ipsrld) CASE(UD_Ipsrlq) CASE(UD_Ipsubb) CASE(UD_Ipsubw) CASE(UD_Ipsubd)
		CASE(UD_Ipsubsb) CASE(UD_Ipsubsw) CASE(UD_Ipsubusb) CASE(UD_Ipsubusw) CASE(UD_Ipunpckhbw) CASE(UD_Ipunpckhwd)
		CASE(UD_Ipunpckhdq) CASE(UD_Ipunpcklbw) CASE(UD_Ipunpcklwd) CASE(UD_Ipunpckldq) CASE(UD_Ipi2fw) CASE(UD_Ipi2fd)
		CASE(UD_Ipf2iw) CASE(UD_Ipf2id) CASE(UD_Ipfnacc) CASE(UD_Ipfpnacc) CASE(UD_Ipfcmpge) CASE(UD_Ipfmin)
		CASE(UD_Ipfrcp) CASE(UD_Ipfrsqrt) CASE(UD_Ipfsub) CASE(UD_Ipfadd) CASE(UD_Ipfcmpgt) CASE(UD_Ipfmax)
		CASE(UD_Ipfrcpit1) CASE(UD_Ipfrsqit1) CASE(UD_Ipfsubr) CASE(UD_Ipfacc) CASE(UD_Ipfcmpeq) CASE(UD_Ipfmul)
		CASE(UD_Ipfrcpit2) CASE(UD_Ipmulhrw) CASE(UD_Ipswapd) CASE(UD_Ipavgusb) CASE(UD_Ipush) CASE(UD_Ipusha)
		CASE(UD_Ipushad) CASE(UD_Ipushfw) CASE(UD_Ipushfd) CASE(UD_Ipushfq) CASE(UD_Ipxor) CASE(UD_Ircl)
		CASE(UD_Ircr) CASE(UD_Irol) CASE(UD_Iror) CASE(UD_Ircpps) CASE(UD_Ircpss) CASE(UD_Irdmsr)
		CASE(UD_Irdpmc) CASE(UD_Irdtsc) CASE(UD_Irdtscp) CASE(UD_Irepne) CASE(UD_Irep) CASE(UD_Iret)
		CASE(UD_Iretf) CASE(UD_Irsm) CASE(UD_Irsqrtps) CASE(UD_Irsqrtss) CASE(UD_Isahf) CASE(UD_Isalc)
		CASE(UD_Isar) CASE(UD_Ishl) CASE(UD_Ishr) CASE(UD_Isbb) CASE(UD_Iscasb) CASE(UD_Iscasw)
		CASE(UD_Iscasd) CASE(UD_Iscasq) CASE(UD_Iseto) CASE(UD_Isetno) CASE(UD_Isetb) CASE(UD_Isetnb)
		CASE(UD_Isetz) CASE(UD_Isetnz) CASE(UD_Isetbe) CASE(UD_Iseta) CASE(UD_Isets) CASE(UD_Isetns)
		CASE(UD_Isetp) CASE(UD_Isetnp) CASE(UD_Isetl) CASE(UD_Isetge) CASE(UD_Isetle) CASE(UD_Isetg)
		CASE(UD_Isfence) CASE(UD_Isgdt) CASE(UD_Ishld) CASE(UD_Ishrd) CASE(UD_Ishufpd) CASE(UD_Ishufps)
		CASE(UD_Isidt) CASE(UD_Isldt) CASE(UD_Ismsw) CASE(UD_Isqrtps) CASE(UD_Isqrtpd) CASE(UD_Isqrtsd)
		CASE(UD_Isqrtss) CASE(UD_Istc) CASE(UD_Istd) CASE(UD_Istgi) CASE(UD_Isti) CASE(UD_Iskinit)
		CASE(UD_Istmxcsr) CASE(UD_Istosb) CASE(UD_Istosw) CASE(UD_Istosd) CASE(UD_Istosq) CASE(UD_Istr)
		CASE(UD_Isub) CASE(UD_Isubpd) CASE(UD_Isubps) CASE(UD_Isubsd) CASE(UD_Isubss) CASE(UD_Iswapgs)
		CASE(UD_Isyscall) CASE(UD_Isysenter) CASE(UD_Isysexit) CASE(UD_Isysret) CASE(UD_Itest) CASE(UD_Iucomisd)
		CASE(UD_Iucomiss) CASE(UD_Iud2) CASE(UD_Iunpckhpd) CASE(UD_Iunpckhps) CASE(UD_Iunpcklps) CASE(UD_Iunpcklpd)
		CASE(UD_Iverr) CASE(UD_Iverw) CASE(UD_Ivmcall) CASE(UD_Ivmclear) CASE(UD_Ivmxon) CASE(UD_Ivmptrld)
		CASE(UD_Ivmptrst) CASE(UD_Ivmlaunch) CASE(UD_Ivmresume) CASE(UD_Ivmxoff) CASE(UD_Ivmread) CASE(UD_Ivmwrite)
		CASE(UD_Ivmrun) CASE(UD_Ivmmcall) CASE(UD_Ivmload) CASE(UD_Ivmsave) CASE(UD_Iwait) CASE(UD_Iwbinvd)
		CASE(UD_Iwrmsr) CASE(UD_Ixadd) CASE(UD_Ixchg) CASE(UD_Ixlatb) CASE(UD_Ixor) CASE(UD_Ixorpd)
		CASE(UD_Ixorps) CASE(UD_Ixcryptecb) CASE(UD_Ixcryptcbc) CASE(UD_Ixcryptctr) CASE(UD_Ixcryptcfb) CASE(UD_Ixcryptofb)
		CASE(UD_Ixsha1) CASE(UD_Ixsha256) CASE(UD_Ixstore) CASE(UD_Imovdqa) CASE(UD_Imovdq2q) CASE(UD_Imovdqu)
		CASE(UD_Imovq2dq) CASE(UD_Ipaddq) CASE(UD_Ipsubq) CASE(UD_Ipmuludq) CASE(UD_Ipshufhw) CASE(UD_Ipshuflw)
		CASE(UD_Ipshufd) CASE(UD_Ipslldq) CASE(UD_Ipsrldq) CASE(UD_Ipunpckhqdq) CASE(UD_Ipunpcklqdq) CASE(UD_Iaddsubpd)
		CASE(UD_Iaddsubps) CASE(UD_Ihaddpd) CASE(UD_Ihaddps) CASE(UD_Ihsubpd) CASE(UD_Ihsubps) CASE(UD_Imovddup)
		CASE(UD_Imovshdup) CASE(UD_Imovsldup) CASE(UD_Ipabsb) CASE(UD_Ipabsw) CASE(UD_Ipabsd) CASE(UD_Ipsignb)
		CASE(UD_Iphaddw) CASE(UD_Iphaddd) CASE(UD_Iphaddsw) CASE(UD_Ipmaddubsw) CASE(UD_Iphsubw) CASE(UD_Iphsubd)
		CASE(UD_Iphsubsw) CASE(UD_Ipsignd) CASE(UD_Ipsignw) CASE(UD_Ipmulhrsw) CASE(UD_Ipalignr) CASE(UD_Ipblendvb)
		CASE(UD_Ipmuldq) CASE(UD_Ipminsb) CASE(UD_Ipminsd) CASE(UD_Ipminuw) CASE(UD_Ipminud) CASE(UD_Ipmaxsb)
		CASE(UD_Ipmaxsd) CASE(UD_Ipmaxud) CASE(UD_Ipmulld) CASE(UD_Iphminposuw) CASE(UD_Iroundps) CASE(UD_Iroundpd)
		CASE(UD_Iroundss) CASE(UD_Iroundsd) CASE(UD_Iblendpd) CASE(UD_Ipblendw) CASE(UD_Iblendps) CASE(UD_Iblendvpd)
		CASE(UD_Iblendvps) CASE(UD_Idpps) CASE(UD_Idppd) CASE(UD_Impsadbw) CASE(UD_Iextractps)
		default:
			return "???";
	}
}

char const *
Udis86Helper::typeToString(ud_type type)
{
	switch (type) {
	CASE(UD_NONE)

	/* 8 bit GPRs */
	CASE(UD_R_AL) CASE(UD_R_CL) CASE(UD_R_DL) CASE(UD_R_BL)
	CASE(UD_R_AH) CASE(UD_R_CH) CASE(UD_R_DH) CASE(UD_R_BH)
	CASE(UD_R_SPL) CASE(UD_R_BPL) CASE(UD_R_SIL) CASE(UD_R_DIL)
	CASE(UD_R_R8B) CASE(UD_R_R9B) CASE(UD_R_R10B) CASE(UD_R_R11B)
	CASE(UD_R_R12B) CASE(UD_R_R13B) CASE(UD_R_R14B) CASE(UD_R_R15B)

	/* 16 bit GPRs */
	CASE(UD_R_AX) CASE(UD_R_CX) CASE(UD_R_DX) CASE(UD_R_BX)
	CASE(UD_R_SP) CASE(UD_R_BP) CASE(UD_R_SI) CASE(UD_R_DI)
	CASE(UD_R_R8W) CASE(UD_R_R9W) CASE(UD_R_R10W) CASE(UD_R_R11W)
	CASE(UD_R_R12W) CASE(UD_R_R13W) CASE(UD_R_R14W) CASE(UD_R_R15W)

	/* 32 bit GPRs */
	CASE(UD_R_EAX) CASE(UD_R_ECX) CASE(UD_R_EDX) CASE(UD_R_EBX)
	CASE(UD_R_ESP) CASE(UD_R_EBP) CASE(UD_R_ESI) CASE(UD_R_EDI)
	CASE(UD_R_R8D) CASE(UD_R_R9D) CASE(UD_R_R10D) CASE(UD_R_R11D)
	CASE(UD_R_R12D) CASE(UD_R_R13D) CASE(UD_R_R14D) CASE(UD_R_R15D)

	/* 64 bit GPRs */
	CASE(UD_R_RAX) CASE(UD_R_RCX) CASE(UD_R_RDX) CASE(UD_R_RBX)
	CASE(UD_R_RSP) CASE(UD_R_RBP) CASE(UD_R_RSI) CASE(UD_R_RDI)
	CASE(UD_R_R8) CASE(UD_R_R9) CASE(UD_R_R10) CASE(UD_R_R11)
	CASE(UD_R_R12) CASE(UD_R_R13) CASE(UD_R_R14) CASE(UD_R_R15)

	/* segment registers */
	CASE(UD_R_ES) CASE(UD_R_CS) CASE(UD_R_SS) CASE(UD_R_DS)
	CASE(UD_R_FS) CASE(UD_R_GS)

	/* control registers*/
	CASE(UD_R_CR0) CASE(UD_R_CR1) CASE(UD_R_CR2) CASE(UD_R_CR3)
	CASE(UD_R_CR4) CASE(UD_R_CR5) CASE(UD_R_CR6) CASE(UD_R_CR7)
	CASE(UD_R_CR8) CASE(UD_R_CR9) CASE(UD_R_CR10) CASE(UD_R_CR11)
	CASE(UD_R_CR12) CASE(UD_R_CR13) CASE(UD_R_CR14) CASE(UD_R_CR15)

	/* debug registers */
	CASE(UD_R_DR0) CASE(UD_R_DR1) CASE(UD_R_DR2) CASE(UD_R_DR3)
	CASE(UD_R_DR4) CASE(UD_R_DR5) CASE(UD_R_DR6) CASE(UD_R_DR7)
	CASE(UD_R_DR8) CASE(UD_R_DR9) CASE(UD_R_DR10) CASE(UD_R_DR11)
	CASE(UD_R_DR12) CASE(UD_R_DR13) CASE(UD_R_DR14) CASE(UD_R_DR15)

	/* mmx registers */
	CASE(UD_R_MM0) CASE(UD_R_MM1) CASE(UD_R_MM2) CASE(UD_R_MM3)
	CASE(UD_R_MM4) CASE(UD_R_MM5) CASE(UD_R_MM6) CASE(UD_R_MM7)

	/* x87 registers */
	CASE(UD_R_ST0) CASE(UD_R_ST1) CASE(UD_R_ST2) CASE(UD_R_ST3)
	CASE(UD_R_ST4) CASE(UD_R_ST5) CASE(UD_R_ST6) CASE(UD_R_ST7)

	/* extended multimedia registers */
	CASE(UD_R_XMM0) CASE(UD_R_XMM1) CASE(UD_R_XMM2) CASE(UD_R_XMM3)
	CASE(UD_R_XMM4) CASE(UD_R_XMM5) CASE(UD_R_XMM6) CASE(UD_R_XMM7)
	CASE(UD_R_XMM8) CASE(UD_R_XMM9) CASE(UD_R_XMM10) CASE(UD_R_XMM11)
	CASE(UD_R_XMM12) CASE(UD_R_XMM13) CASE(UD_R_XMM14) CASE(UD_R_XMM15)

	CASE(UD_R_RIP)

	/* Operand Types */
	CASE(UD_OP_REG) CASE(UD_OP_MEM) CASE(UD_OP_PTR) CASE(UD_OP_IMM)
	CASE(UD_OP_JIMM) CASE(UD_OP_CONST)
		default:
			return "???";
	}
}
#undef CASE

fail::GPRegisterId
Udis86Helper::udisGPRToFailBochsGPR(ud_type_t udisReg, uint64_t& bitmask)
{
#define REG_CASE(UDREG, FAILREG, BITMASK) case UD_R_##UDREG: bitmask = BITMASK; return fail::RID_##FAILREG;
	switch (udisReg) {
#if BX_SUPPORT_X86_64 // 64 bit register id's:
	// TODO
#else
	// 8 bit GPRs
	REG_CASE(AL, EAX, 0xff)
	REG_CASE(BL, EBX, 0xff)
	REG_CASE(CL, ECX, 0xff)
	REG_CASE(DL, EDX, 0xff)
	REG_CASE(SPL, ESP, 0xff)
	REG_CASE(BPL, EBP, 0xff)
	REG_CASE(SIL, ESI, 0xff)
	REG_CASE(DIL, EDI, 0xff)
	// (R8B-R15B)
	REG_CASE(AH, EAX, 0xff00)
	REG_CASE(BH, EBX, 0xff00)
	REG_CASE(CH, ECX, 0xff00)
	REG_CASE(DH, EDX, 0xff00)
	// 16 bit GPRs
	REG_CASE(AX, EAX, 0xffff)
	REG_CASE(BX, EBX, 0xffff)
	REG_CASE(CX, ECX, 0xffff)
	REG_CASE(DX, EDX, 0xffff)
	REG_CASE(SP, ESP, 0xffff)
	REG_CASE(BP, EBP, 0xffff)
	REG_CASE(SI, ESI, 0xffff)
	REG_CASE(DI, EDI, 0xffff)
	// (R8W-R15W)
	// 32 bit GPRs
	REG_CASE(EAX, EAX, 0xffffffff)
	REG_CASE(EBX, EBX, 0xffffffff)
	REG_CASE(ECX, ECX, 0xffffffff)
	REG_CASE(EDX, EDX, 0xffffffff)
	REG_CASE(ESP, ESP, 0xffffffff)
	REG_CASE(EBP, EBP, 0xffffffff)
	REG_CASE(ESI, ESI, 0xffffffff)
	REG_CASE(EDI, EDI, 0xffffffff)
	// missing: segment, control, debug, mmx, x86, xmm registers, EIP
#endif
	default:
		return fail::RID_LAST_GP_ID;
	}
#undef REG_CASE
}

void Udis86Helper::initOpcodeModMap()
{
	// The mentioned operands only include general-purpose registers.
	// Operand order in Intel style (destinations, then sources)
	m[UD_Iadd].push_back(ModificationInfo(INOUT, IN));
	m[UD_Iadc].push_back(ModificationInfo(INOUT, IN));
	m[UD_Iand].push_back(ModificationInfo(INOUT, IN));
	m[UD_Icall].push_back(ModificationInfo(IN));
	m[UD_Icall].back().addImplicit(UD_R_ESP, INOUT); // reads/modifies ESP
	m[UD_Icmp].push_back(ModificationInfo(IN, IN));
	m[UD_Iidiv].push_back(ModificationInfo(IN));
	m[UD_Iidiv].back().addImplicit(UD_R_EAX, INOUT); // FIXME implicit op size depends on explicit op size
	m[UD_Iidiv].back().addImplicit(UD_R_EDX, INOUT); // FIXME implicit op size depends on explicit op size
	m[UD_Idec].push_back(ModificationInfo(INOUT));
	m[UD_Iimul].push_back(ModificationInfo(INOUT, IN)); // two-operand version
	m[UD_Iimul].push_back(ModificationInfo(OUT, IN, IN)); // three-operand version
	m[UD_Iinc].push_back(ModificationInfo(INOUT));
	m[UD_Ija].push_back(ModificationInfo(IN));
	m[UD_Ijae].push_back(ModificationInfo(IN));
	m[UD_Ijb].push_back(ModificationInfo(IN));
	m[UD_Ijbe].push_back(ModificationInfo(IN));
	m[UD_Ijg].push_back(ModificationInfo(IN));
	m[UD_Ijge].push_back(ModificationInfo(IN));
	m[UD_Ijl].push_back(ModificationInfo(IN));
	m[UD_Ijle].push_back(ModificationInfo(IN));
	m[UD_Ijmp].push_back(ModificationInfo(IN));
	m[UD_Ijno].push_back(ModificationInfo(IN));
	m[UD_Ijnp].push_back(ModificationInfo(IN));
	m[UD_Ijns].push_back(ModificationInfo(IN));
	m[UD_Ijnz].push_back(ModificationInfo(IN));
	m[UD_Ijs].push_back(ModificationInfo(IN));
	m[UD_Ijz].push_back(ModificationInfo(IN));
	m[UD_Ilea].push_back(ModificationInfo(OUT, IN));
	m[UD_Ileave].push_back(ModificationInfo()); // MOV %ebp, %esp; POP %ebp
	m[UD_Ileave].back().addImplicit(UD_R_EBP, INOUT);
	m[UD_Ileave].back().addImplicit(UD_R_ESP, OUT); // INOUT, if not considered atomic
	m[UD_Imov].push_back(ModificationInfo(OUT, IN));
	m[UD_Imovsx].push_back(ModificationInfo(OUT, IN));
	m[UD_Imovzx].push_back(ModificationInfo(OUT, IN));
	m[UD_Ineg].push_back(ModificationInfo(INOUT));
	m[UD_Inop].push_back(ModificationInfo());
	m[UD_Inot].push_back(ModificationInfo(INOUT));
	m[UD_Ior].push_back(ModificationInfo(INOUT, IN));
	m[UD_Iout].push_back(ModificationInfo(IN, IN));
	m[UD_Ipop].push_back(ModificationInfo(OUT));
	m[UD_Ipop].back().addImplicit(UD_R_ESP, INOUT); // reads/modifies ESP
	m[UD_Ipush].push_back(ModificationInfo(IN));
	m[UD_Ipush].back().addImplicit(UD_R_ESP, INOUT); // reads/modifies ESP
	m[UD_Iret].push_back(ModificationInfo());
	m[UD_Iret].back().addImplicit(UD_R_ESP, INOUT); // reads/modifies ESP
	//m[UD_Isalc].push_back(ModificationInfo(INOUT, IN)); untested
	m[UD_Isar].push_back(ModificationInfo(INOUT, IN));
	m[UD_Iseta].push_back(ModificationInfo(OUT));
	//m[UD_Isetae].push_back(ModificationInfo(OUT));
	m[UD_Isetb].push_back(ModificationInfo(OUT));
	m[UD_Isetbe].push_back(ModificationInfo(OUT));
	//m[UD_Isetc].push_back(ModificationInfo(OUT));
	//m[UD_Isete].push_back(ModificationInfo(OUT));
	m[UD_Isetg].push_back(ModificationInfo(OUT));
	m[UD_Isetge].push_back(ModificationInfo(OUT));
	m[UD_Isetl].push_back(ModificationInfo(OUT));
	m[UD_Isetle].push_back(ModificationInfo(OUT));
	//m[UD_Isetna].push_back(ModificationInfo(OUT));
	//m[UD_Isetnae].push_back(ModificationInfo(OUT));
	m[UD_Isetnb].push_back(ModificationInfo(OUT));
	//m[UD_Isetnbe].push_back(ModificationInfo(OUT));
	//m[UD_Isetnc].push_back(ModificationInfo(OUT));
	//m[UD_Isetne].push_back(ModificationInfo(OUT));
	//m[UD_Isetng].push_back(ModificationInfo(OUT));
	//m[UD_Isetnge].push_back(ModificationInfo(OUT));
	//m[UD_Isetnl].push_back(ModificationInfo(OUT));
	//m[UD_Isetnle].push_back(ModificationInfo(OUT));
	m[UD_Isetno].push_back(ModificationInfo(OUT));
	m[UD_Isetnp].push_back(ModificationInfo(OUT));
	m[UD_Isetns].push_back(ModificationInfo(OUT));
	m[UD_Isetnz].push_back(ModificationInfo(OUT));
	m[UD_Iseto].push_back(ModificationInfo(OUT));
	m[UD_Isetp].push_back(ModificationInfo(OUT));
	//m[UD_Isetpe].push_back(ModificationInfo(OUT));
	//m[UD_Isetpo].push_back(ModificationInfo(OUT));
	m[UD_Isets].push_back(ModificationInfo(OUT));
	m[UD_Isetz].push_back(ModificationInfo(OUT));
	m[UD_Ishl].push_back(ModificationInfo(INOUT, IN));
	m[UD_Ishr].push_back(ModificationInfo(INOUT, IN));
	m[UD_Isub].push_back(ModificationInfo(INOUT, IN));
	m[UD_Itest].push_back(ModificationInfo(IN, IN));
	m[UD_Ixor].push_back(ModificationInfo(INOUT, IN));
}

void Udis86Helper::inOutRegisters(UDRegisterSet& in, UDRegisterSet& out)
{
	unsigned nops = operandCount();

	in.clear();
	out.clear();

	OpcodeModificationMap::const_iterator it = m.find(ud->mnemonic);
	if (it == m.end()) {
		std::cerr << __func__ << ": Unknown mnemonic "
			<< mnemonicToString(ud->mnemonic)
		    << " (0x" << std::hex << ((int) ud->mnemonic) << ") :: "
			<< ud_insn_asm(ud) << std::endl;
		// continue to fallback solution below
	} else {
		std::vector<ModificationInfo> const &v = it->second;
		for (std::vector<ModificationInfo>::const_iterator it = v.begin();
			 it != v.end(); ++it) {
			if (it->dir.size() != nops) {
				continue;
			}
			// found a match, transfer information
			for (unsigned i = 0; i < nops; ++i) {
				if (it->dir[i] == IN || it->dir[i] == INOUT) {
					transferRegisters(in, in, i);
				}
				if (it->dir[i] == OUT || it->dir[i] == INOUT) {
					transferRegisters(out, in, i);
				}
			}
			// handle implicit registers, if any
			for (std::vector<std::pair<ud_type, OperandDirection> >::const_iterator impl_it = it->implicitOperands.begin();
			     impl_it != it->implicitOperands.end(); ++impl_it) {
				if (impl_it->second == IN || impl_it->second == INOUT) {
					in.insert(impl_it->first);
				}
				if (impl_it->second == OUT || impl_it->second == INOUT) {
					out.insert(impl_it->first);
				}
			}
			// handle special cases
			handleSpecialCases(in, out);
			return;
		}
		// if none found, continue to fallback solution below
	}

	// Let's do our best and simply include *all* registers udis86 mentions.
	// This is most certainly wrong, but (implicit registers aside) a
	// conservative, safe fallback.
	for (unsigned i = 0; i < nops; ++i) {
		transferRegisters(in, in, i);
		transferRegisters(out, in, i);
	}
}

void
Udis86Helper::transferRegisters(UDRegisterSet& target_set, UDRegisterSet& in_set, int idx)
{
	if (ud->mnemonic == UD_Isub) {
//		std::cerr << "found SUB";
	}
	switch (ud->operand[idx].type) {
	case UD_OP_REG:
		target_set.insert(ud->operand[idx].base);
		/* ud->operand[opno].size? */
		break;

	case UD_OP_MEM:
		/*
		 * Address may be calculated from a base and an index register (plus a
		 * scale, but this * is a constant, which we don't care about.
		 */
		if (ud->operand[idx].base != UD_NONE) {
			in_set.insert(ud->operand[idx].base);
		}
		if (ud->operand[idx].index != UD_NONE) {
			in_set.insert(ud->operand[idx].index);
		}
		break;

	case UD_OP_IMM:
	case UD_OP_CONST:
	case UD_NONE:
	case UD_OP_JIMM:
		/*
		 * Nothing to do here.
		 */
		break;

	default:
		std::cerr << __func__ << ": unhandled type " << ud->operand[idx].type << std::endl;
		break;
	}
}

void Udis86Helper::handleSpecialCases(UDRegisterSet& in, UDRegisterSet& out)
{
	if ((ud->mnemonic == UD_Ixor) &&
	    (ud->operand[0].type == UD_OP_REG) && (ud->operand[1].type == UD_OP_REG) &&
	    (ud->operand[0].base == ud->operand[1].base)) {
		in.clear(); // Hard-coded. We only have 2 operands here. Simply remove the read.
	}
}
