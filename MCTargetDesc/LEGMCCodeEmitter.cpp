
#include "MCTargetDesc/LEGMCTargetDesc.h"
#include "MCTargetDesc/LEGFixupKinds.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCRegisterInfo.h"

#include "llvm/Support/EndianStream.h"

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");
STATISTIC(MCNumFixups, "Number of MC fixups created");

namespace {
class LEGMCCodeEmitter : public MCCodeEmitter {
	LEGMCCodeEmitter(const LEGMCCodeEmitter&) = delete;
	void operator=(const LEGMCCodeEmitter&) = delete;
	MCContext &Ctx;
	MCInstrInfo const &MCII;

public:
	LEGMCCodeEmitter(MCContext &ctx, MCInstrInfo const &MCII)
		: Ctx(ctx), MCII(MCII) {}
	~LEGMCCodeEmitter() override = default;

	//void emitInstruction(uint64_t val, unsigned size, const MCSubtargetInfo &STI, raw_ostream &OS) const;

	// useb by addrbo
	uint64_t getAddrModeRegImm(const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const;
	uint64_t getImmOpValSr2(const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const;
	void emitImmediate(const MCInst &MI, const MCOperand &ImmOp, raw_ostream &OS, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const;
	void emitJmpEnc(const MCInst &MI, raw_ostream &OS, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const;

	void encodeInstruction(const MCInst &MI, raw_ostream &OS, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const override;

	unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const;
	// tbgen generated
	uint64_t getBinaryCodeForInstr(const MCInst &MI, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const;
};
}

MCCodeEmitter *llvm::createLEGMCCodeEmitter(const MCInstrInfo &MCII, MCContext &Ctx) {
	return new LEGMCCodeEmitter(Ctx, MCII);
}

unsigned LEGMCCodeEmitter::getMachineOpValue(const MCInst &MI,
	                                         const MCOperand &MO,
	                                         SmallVectorImpl<MCFixup> &Fixups,
	                                         const MCSubtargetInfo &STI) const {
	// these two lines are pretty universal
	if (MO.isReg())
		return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());
	if (MO.isImm())
		return static_cast<unsigned>(MO.getImm());

	MO.dump();
	llvm_unreachable("unhandled expression!");
	return 0;
}
/*
void LEGMCCodeEmitter::emitInstruction(uint64_t val, unsigned int size, const MCSubtargetInfo &STI, raw_ostream &OS) const {
	uint32_t v = val;
	support::endian::write(OS, v, support::endianness::little);
}
*/

uint64_t LEGMCCodeEmitter::getAddrModeRegImm(const MCInst &MI, unsigned int OpNo, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const {
	uint64_t ret = 0;
	ret |= getMachineOpValue(MI, MI.getOperand(OpNo), Fixups, STI) << 16;
	ret |= MI.getOperand(OpNo + 1).getImm() & 0xffff;
	return ret;
}

uint64_t LEGMCCodeEmitter::getImmOpValSr2(const MCInst &MI, unsigned int OpNo, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const {
	const MCOperand &MO = MI.getOperand(OpNo);

	if (MO.isImm()) {
		unsigned Imm = MO.getImm();
		assert((Imm & 3) == 0 && "least 2 bit are not zero");
		return Imm >> 2;
	}

	const MCExpr *Expr = MO.getExpr();
	MCExpr::ExprKind Kind = Expr->getKind();
	unsigned opcode = MI.getOpcode();

	switch (opcode) {
	case LEG::BR:
	case LEG::BR_ENC:
	case LEG::BR_NOENC:
		Fixups.push_back(MCFixup::create(0, Expr, MCFixupKind(LEG::fixup_leg_pcrel_jmp27), MI.getLoc()));
		++MCNumFixups;
		break;
	case LEG::BRCC:
	case LEG::BRCC_ENC:
	case LEG::BRCC_NOENC:
		Fixups.push_back(MCFixup::create(0, Expr, MCFixupKind(LEG::fixup_leg_pcrel_jmp22), MI.getLoc()));
		++MCNumFixups;
		break;
	}

	return 0;
}

void LEGMCCodeEmitter::emitImmediate(const MCInst &MI, const MCOperand &ImmOp, raw_ostream &OS, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const {

	auto endian = STI.getFeatureBits()[LEG::FeatureBIGENDIAN] ? support::big : support::little;

	if (ImmOp.isImm()) {
		support::endian::write(OS, ImmOp.getImm(), endian);
		return;
	}

	ImmOp.dump();
	assert(ImmOp.isExpr() && "expected expression");
	const MCExpr *Expr = ImmOp.getExpr();
	MCExpr::ExprKind Kind = Expr->getKind();

	Fixups.push_back(MCFixup::create(0, Expr, MCFixupKind(LEG::fixup_leg_imm64), MI.getLoc()));
	++MCNumFixups;

	support::endian::write(OS, (uint64_t) 0, endian);
	return;
}

void LEGMCCodeEmitter::emitJmpEnc(const MCInst &MI, raw_ostream &OS, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const {
	if (!STI.getFeatureBits()[LEG::FeatureInsEncryption])
		return;

	support::endian::write(OS, (uint32_t) 0, support::little);
	return;
}

void LEGMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const {
	uint32_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
	auto endian = STI.getFeatureBits()[LEG::FeatureBIGENDIAN] ? support::big : support::little;
	support::endian::write(OS, Bits, endian);
	switch (MI.getOpcode()) {
	default:
		break;
	case LEG::LDI:
		emitImmediate(MI, MI.getOperand(1), OS, Fixups, STI);
		break;
	case LEG::BR_ENC:
	case LEG::BRCC_ENC:
		emitJmpEnc(MI, OS, Fixups, STI);
		break;
	}
	//if (MI.getOpcode() == LEG::LDI) {
	//	emitImmediate(MI, MI.getOperand(1), OS, Fixups, STI);
	//}
	++MCNumEmitted;
}

#include "LEGGenMCCodeEmitter.inc"