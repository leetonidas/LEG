let Namespace = "LEG" in {
class LEGReg<bits<5> Enc, string n, list<Register> subregs = [], list<string> altNames = []> : Register<n> {
	let HWEncoding{4-0} = Enc;
	//let SubRegs = subregs;
	let AltNames = altNames;
}
def RegNamesRaw : RegAltNameIndex {
	let FallbackRegAltNameIndex = NoRegAltName;
}
/*

def sub_8 : SubRegIndex<8>;
def sub_16 : SubRegIndex<16>;
def sub_32 : SubRegIndex<32>;
foreach Index = 0-28 in {
	def R#Index#b : LEGReg< Index, "r"#Index#"b" >, DwarfRegNum<[Index]>;
}
let RegAltNameIndices = [RegNamesRaw] in {
	def BPb : LEGReg< 29, "bpb", [], ["r29b"] >, DwarfRegNum<[29]>;
	def SPb : LEGReg< 30, "spb", [], ["r30b"] >, DwarfRegNum<[30]>;
	def PCb : LEGReg< 31, "pcb", [], ["r31b"] >, DwarfRegNum<[31]>;
}

let SubRegIndices = [sub_8] in {
foreach Index = 0-28 in {
	def R#Index#q : LEGReg< Index, "r" # Index # "q" , [ !cast<LEGReg>("R"#Index#"b") ] >, DwarfRegNum<[Index]>;
}
let RegAltNameIndices = [RegNamesRaw] in {
	def BPq : LEGReg< 29, "bpq", [BPb], ["r29q"] >, DwarfRegNum<[29]>;
	def SPq : LEGReg< 30, "spq", [SPb], ["r30q"] >, DwarfRegNum<[30]>;
	def PCq : LEGReg< 31, "pcq", [PCb], ["r31q"] >, DwarfRegNum<[31]>;
}
}

let SubRegIndices = [sub_16] in {
foreach Index = 0-28 in {
	def R#Index#h : LEGReg< Index, "r"#Index#"h", [!cast<LEGReg>("R"#Index#"q")] >, DwarfRegNum<[Index]>;
}
let RegAltNameIndices = [RegNamesRaw] in {
	def BPh : LEGReg< 29, "bph", [BPq], ["r29h"] >, DwarfRegNum<[29]>;
	def SPh : LEGReg< 30, "sph", [SPq], ["r30h"] >, DwarfRegNum<[30]>;
	def PCh : LEGReg< 31, "pch", [PCq], ["r31h"] >, DwarfRegNum<[31]>;
}
}

let SubRegIndices = [sub_32] in {
*/
foreach Index = 0-28 in {
	def R#Index : LEGReg< Index, "r"#Index, [/*!cast<LEGReg>("R"#Index#"h")*/] >, DwarfRegNum<[Index]>;
}
let RegAltNameIndices = [RegNamesRaw] in {
	def BP : LEGReg< 29, "bp", [/*BPh*/], ["r29"] >, DwarfRegNum<[29]>;
	def SP : LEGReg< 30, "sp", [/*SPh*/], ["r30"] >, DwarfRegNum<[30]>;
	def PC : LEGReg< 31, "pc", [/*PCh*/], ["r31"] >, DwarfRegNum<[31]>;
}
//}

def GPR : RegisterClass<"LEG", [i64], 64,
	(add (sequence "R%u", 0, 28), BP, SP, PC)>;

def GPRnopc : RegisterClass<"LEG", [i64], 64,
	(add (sequence "R%u", 0, 28), BP, SP)>;
/*
def GPR32 : RegisterClass<"LEG", [i32], 32,
	(add (sequence "R%uh", 0, 28), BPh, SPh, PCh)>;

def GPR32nopc : RegisterClass<"LEG", [i32], 32,
	(add (sequence "R%uh", 0, 28), BPh, SPh)>;

def GPR16 : RegisterClass<"LEG", [i16], 16,
	(add (sequence "R%uq", 0, 28), BPq, SPq, PCq)>;

def GPR16nopc : RegisterClass<"LEG", [i16], 16,
	(add (sequence "R%uq", 0, 28), BPq, SPq)>;

def GPR8 : RegisterClass<"LEG", [i8], 8,
	(add (sequence "R%ub", 0, 28), BPb, SPb, PCb)>;

def GPR8nopc : RegisterClass<"LEG", [i8], 8,
	(add (sequence "R%ub", 0, 28), BPb, SPb)>;
*/
}