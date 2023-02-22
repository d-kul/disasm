#include <iostream>
#include <iomanip>
#include <fstream>
#include <unordered_map>
#define EI_NIDENT 16
using namespace std;

typedef uint32_t Word;
typedef uint16_t Half;
typedef uint32_t Addr;
typedef uint32_t Off;

typedef struct {
    unsigned char   e_ident[EI_NIDENT];
    Half    e_type;
    Half    e_machine;
    Word    e_version;
    Addr    e_entry;
    Off     e_phoff;
    Off     e_shoff;
    Word    e_flags;
    Half    e_ehsize;
    Half    e_phentsize;
    Half    e_phnum;
    Half    e_shentsize;
    Half    e_shnum;
    Half    e_shstrndx;
} ElfHeader;

typedef struct {
    Word    p_type;
    Off     p_offset;
    Addr	p_vaddr;
    Addr	p_paddr;
    Word	p_filesz;
    Word	p_memsz;
    Word	p_flags;
    Word    p_align;
} ProgramHeader;

typedef struct {
    Word	sh_name;
    Word	sh_type;
    Word	sh_flags;
    Addr	sh_addr;
    Off	    sh_offset;
    Word	sh_size;
    Word	sh_link;
    Word	sh_info;
    Word	sh_addralign;
    Word	sh_entsize;
} SectionHeader;

typedef struct {
    Word    st_name;
    Addr    st_value;
    Word    st_size;
    unsigned char   st_info;
    unsigned char   st_other;
    Half    st_shndx;
} Symbol;

void readHeader(ifstream& inputFile, ElfHeader& header) {
    if (!inputFile.is_open()) {
        return throw exception();
    }
#ifdef NDEBUG
    cout << "File header...\t\t";
#endif
    inputFile.seekg(0, ios_base::beg);
    inputFile.read((char *)&header, sizeof(ElfHeader));

    unsigned char ident[] = {
//      e_ident
        0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//      e_type      e_machine   e_version
        0x02, 0x00, 0xf3, 0x00, 0x01, 0x00, 0x00, 0x00,
    };
    
    for (int i = 0; i < EI_NIDENT + 8; i++) {
        if (((unsigned char *)&header)[i] != ident[i]) {
#ifdef NDEBUG
    cout << "Gone wrong." << endl;
#endif
            return throw exception();
        }
    }
#ifdef NDEBUG
    cout << "Done!" << endl;
#endif
}

void readProgramHeader(ifstream& inputFile, ElfHeader& header, ProgramHeader * programHeader) {
    if (!inputFile.is_open()) {
        return throw exception();
    }
#ifdef NDEBUG
    cout << "Program header...\t";
#endif
    inputFile.seekg(header.e_phoff, ios_base::beg);
    for (unsigned short phidx = 0; phidx < header.e_phnum; phidx++) {
        inputFile.read((char *)(programHeader+phidx), sizeof(ProgramHeader));
    }
#ifdef NDEBUG
    cout << "Done!" << endl;
#endif
}

void readSectionHeader(ifstream& inputFile, ElfHeader& header, SectionHeader * sectionHeader) {
    if (!inputFile.is_open()) {
        return throw exception();
    }
#ifdef NDEBUG
    cout << "Section header...\t";
#endif
    inputFile.seekg(header.e_shoff, ios_base::beg);
    for (unsigned short shidx = 0; shidx < header.e_shnum; shidx++) {
        inputFile.read((char *)(sectionHeader + shidx), sizeof(SectionHeader));
    }
#ifdef NDEBUG
    cout << "Done!" << endl;
#endif
}

void getStringTable(ifstream& inputFile, SectionHeader& section, char * stringBuffer) {
    if (!inputFile.is_open()) {
        return throw exception();
    }
#ifdef NDEBUG
    cout << "String table...\t\t";
#endif
    inputFile.seekg(section.sh_offset, ios_base::beg);
    inputFile.read(stringBuffer, section.sh_size);
#ifdef NDEBUG
    cout << "Done!" << endl;
#endif
}

void getSymbolTable(ifstream& inputFile, SectionHeader& section, Symbol * symbolBuffer) {
    if (!inputFile.is_open()) {
        return throw exception();
    }
#ifdef NDEBUG
    cout << "Symbol table...\t\t";
#endif
    inputFile.seekg(section.sh_offset, ios_base::beg);
    inputFile.read((char *)symbolBuffer, section.sh_size);
#ifdef NDEBUG
    cout << "Done!" << endl;
#endif
}

void getProgBits(ifstream& inputFile, SectionHeader& section, Word * wordBuffer) {
    if (!inputFile.is_open()) {
        return throw exception();
    }
#ifdef NDEBUG
    cout << "Program instructions...\t";
#endif
    inputFile.seekg(section.sh_offset, ios_base::beg);
    inputFile.read((char *)wordBuffer, section.sh_size);
#ifdef NDEBUG
    cout << "Done!" << endl;
#endif
}

void getLabels(Symbol * symbolBuffer, Word symbolAmount, unordered_map<Word, string>& labels, char * symbolStringBuffer) {
    for (Word i = 0; i < symbolAmount; i++) {
        if (((unsigned short)symbolBuffer[i].st_info & 0xf) == 0x2) {
            labels.insert({symbolBuffer[i].st_value, string(symbolStringBuffer + symbolBuffer[i].st_name)});
        }
    }
}

void printSections(ElfHeader& header, char * stringBuffer, SectionHeader sectionHeader[]) {
    string types[] = {
        "NULL", "PROGBITS", "SYMTAB", "STRTAB", "RELA", "HASH", 
        "DYNAMIC", "NOTE", "NOBITS", "REL", "SHLIB", "DYNSYM", 
        "UNKNOWN", "UNKNOWN", "INIT_ARRAY", "FINI_ARRAY", 
        "PREINIT_ARRAY", "GROUP", "SYMTAB_SHNDX", "NUM",
    };
    string flags = "WAX MS";

    const int section_w = 7;
    const int name_w = 18;
    const int type_w = 13;
    const int offset_w = 10;
    const int size_w = 10;

    cout << ' ' << left <<
    setw(section_w) << "Section" << " | " << setw(name_w) << "Name" << " | " << setw(type_w) << "Type" << " | " << setw(offset_w) << "Offset" << " | " << setw(size_w) << "Size" << " | " << setw(flags.size())  << "Flags" << 
    endl << right << setfill('-') << '-' << setw(section_w)  << '-' << "-+-" << setw(name_w) << '-' << "-+-" << setw(type_w) << '-' << "-+-" << setw(offset_w) << '-' << "-+-" << setw(size_w) << '-' << "-+-" << setw(flags.size()) << '-' << '-' <<
    endl << setfill(' ');
    for (Half i = 0; i < header.e_shnum; i++) {
        Word type = sectionHeader[i].sh_type;
        cout << ' ' << setw(section_w - 5) << ' ' << right << dec << 
        '[' << setw(3) << i << ']' << " | " << 
        left << 
        setw(name_w) << (stringBuffer + sectionHeader[i].sh_name) << " | " <<
        hex << uppercase;
        if (type > 0x7fffffff) {
            cout << "USER_" << setfill('0') << setw(7) << (type & 0x0fffffff) << setfill(' ') << setw(type_w - 12) << ' ';
        } else if (type > 0x6fffffff) {
            cout << "PROC_" << setfill('0') << setw(7) << (type & 0x0fffffff) << setfill(' ') << setw(type_w - 12) << ' ';
        } else if (type < 0x14) {
            cout << setw(type_w) << types[type];
        } else {
            cout << setw(type_w) << "UNKNOWN";
        } cout << " | ";
        cout << "0x" << setw(offset_w - 2) << sectionHeader[i].sh_offset << " | " <<
        "0x" << setw(size_w - 2) << sectionHeader[i].sh_size << " | ";

        for (int flagIdx = 0; flagIdx < flags.size(); flagIdx++) {
            if (sectionHeader[i].sh_flags & (1UL << flagIdx)) {
                cout << flags[flagIdx];
            } else {
                cout << ' ';
            }
        }
        cout << ' ' << hex << sectionHeader[i].sh_addr << endl;
    }
}

void printProgram(ofstream& out, Word * wordBuffer, Word wordAmount, Off startOff, char * name, unordered_map<Word, string>& labels) {
    if (!out.is_open()) {
        return throw exception();
    }

    string x[] = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", 
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
    };

    // process labels   
    int lCounter = 0;
    for (Word i = 0; i < wordAmount; i++) {
        Addr addr = startOff + 4*i;
        Addr target;
        Word word = wordBuffer[i];
        if (((word >> 2) & 0b11111) == 0b11011) {
            // J
            // w/ sign extension
            int imm20 = (((int32_t)word) >> 31) << 20;
            int imm12 = word & (0b11111'111 << 12);
            int imm11 = ((word >> 20) & 0b1) << 11;
            int imm1  = ((word << 1) >> 22) << 1;
            target = (int32_t)addr + (imm1 | imm11 | imm12 | imm20);
        } else if (((word >> 2) & 0b11111) == 0b11000) {
            // B
            int imm12 = (((int32_t)word) >> 31) << 12;
            int imm11 = ((word >> 7) & 0b1) << 11;
            int imm5 = ((word << 1) >> 26) << 5;
            int imm1 = (word >> 7) & 0b11110;
            target = (int32_t)addr + (imm1 | imm5 | imm11 | imm12);
        } else continue;
        if (labels.count(target) == 0) {
            labels.insert({target, ("L"+to_string(lCounter++))});
        }
    }

    out << name << endl;
    for (Word i = 0; i < wordAmount; i++) {
        Addr addr = startOff + 4*i;
        if (labels.count(addr)) {
            out << setfill('0') << setw(8) << right << hex << (addr) << " \t<" << labels[addr] << ">:" << endl << setfill(' ');
        }
        Word word = wordBuffer[i];
        if (((word & 0b11) != 0b11) || ((word & 0b11100) == 0b11100)) {
            //  not 32-bit
            continue;
        }
        string instruction;
        
        unsigned short opcode = (word >> 2) & 0b11111;
        unsigned short rd = -1, rs1 = -1, rs2 = -1;
        unsigned short funct3 = (word >> 12) & 0b111, funct7 = word >> 25;
        int32_t imm = 0;
        char type = 0xff;
        Addr target;

        switch (opcode) {
            case 0b01101: 
            case 0b00101: {
                // parse U
                type = 'U';
                instruction = (opcode & 0b01000 ? "lui" : "auipc");
                imm = (word >> 12) << 12;
                rd = (word >> 7) & 0b11111;
                break;
            }
            case 0b11011: {
                type = 'J';
                instruction = "jal";
                // w/ sign extension
                int imm20 = (((int32_t)word) >> 31) << 20;
                int imm12 = word & (0b11111'111 << 12);
                int imm11 = ((word >> 20) & 0b1) << 11;
                int imm1  = ((word << 1) >> 22) << 1;
                imm = imm1 | imm11 | imm12 | imm20;
                rd = (word >> 7) & 0b11111;
                target = (int32_t)addr + imm;
                break;
            }
            case 0b11001: {
                type = 'j';
                if (funct3 != 0b000) {
                    continue;
                }
                instruction = "jalr";
                rd = (word >> 7) & 0b11111;
                rs1 = (word >> 15) & 0b11111;
                imm = ((int32_t)word) >> 20;
                break;
            }
            case 0b11000: {
                // branching
                type = 'B';
                switch (funct3) {
                    case 0b000: { instruction = "beq";  break; }
                    case 0b001: { instruction = "bne";  break; }
                    case 0b100: { instruction = "blt";  break; }
                    case 0b101: { instruction = "bge";  break; }
                    case 0b110: { instruction = "bltu"; break; }
                    case 0b111: { instruction = "bgeu"; break; }
                    default: continue;
                }
                rs1 = (word >> 15) & 0b11111;
                rs2 = (word >> 20) & 0b11111;
                int imm12 = (((int32_t)word) >> 31) << 12;
                int imm11 = ((word >> 7) & 0b1) << 11;
                int imm5 = ((word << 1) >> 26) << 5;
                int imm1 = (word >> 7) & 0b11110;
                imm = imm1 | imm5 | imm11 | imm12;
                target = (int32_t)addr + imm;
                break;
            }
            case 0b00000: {
                // load
                type = 'L';
                rd = (word >> 7) & 0b11111;
                rs1 = (word >> 15) & 0b11111;
                imm = ((int32_t)word) >> 20;
                switch (funct3) {
                    case 0b000: { instruction = "lb";  break; }
                    case 0b001: { instruction = "lh";  break; }
                    case 0b010: { instruction = "lw";  break; }
                    case 0b100: { instruction = "lbu"; break; }
                    case 0b101: { instruction = "lhu"; break; }
                    default: continue;
                }
                break;
            }
            case 0b01000: {
                // store
                type = 'S';
                rs1 = (word >> 15) & 0b11111;
                rs2 = (word >> 20) & 0b11111;
                imm = ((word >> 7) & 0b11111) | ((((int32_t)word) >> 25) << 5);
                switch (funct3) {
                    case 0b000: { instruction = "sb"; break; }
                    case 0b001: { instruction = "sh"; break; }
                    case 0b010: { instruction = "sw"; break; }
                    default: continue;
                }
                break;
            }
            case 0b00100: {
                type = 'I';
                rd = (word >> 7) & 0b11111;
                rs1 = (word >> 15) & 0b11111;
                imm = ((int32_t)word) >> 20;
                switch (funct3) {
                    case 0b000: { instruction = "addi"; break; }
                    case 0b001: { if (funct7 != 0b0000000) continue; instruction = "slli"; break; }
                    case 0b010: { instruction = "slti"; break; }
                    case 0b011: { instruction = "sltiu"; break; }
                    case 0b100: { instruction = "xori"; break; }
                    case 0b101: { 
                        if (funct7 == 0b0000000) instruction = "srli";
                        else if (funct7 == 0b0100000) instruction = "srai";
                        else continue;
                        break; 
                    }
                    case 0b110: { instruction = "ori"; break; }
                    case 0b111: { instruction = "andi"; break; }
                    default: continue;
                }
                break;
            }
            case 0b01100: {
                type = 'R';
                rd = (word >> 7) & 0b11111;
                rs1 = (word >> 15) & 0b11111;
                rs2 = (word >> 20) & 0b11111;
                if      (funct7 == 0b0000000)
                    switch (funct3) {
                        case 0b000: { instruction = "add"; break; }
                        case 0b001: { instruction = "sll"; break; }
                        case 0b010: { instruction = "slt"; break; }
                        case 0b011: { instruction = "sltu";break; }
                        case 0b100: { instruction = "xor"; break; }
                        case 0b101: { instruction = "srl"; break; }
                        case 0b110: { instruction = "or"; break; }
                        case 0b111: { instruction = "and";break; }
                        default: continue;
                    }
                else if (funct7 == 0b0100000)
                    switch (funct3) {
                        case 0b000: { instruction = "sub"; break; }
                        case 0b101: { instruction = "sra"; break; }
                        default: continue;
                    }
                else if (funct7 == 0b0000001)
                    switch (funct3) {
                        case 0b000: { instruction = "mul";   break; }
                        case 0b001: { instruction = "mulh";  break; }
                        case 0b010: { instruction = "mulhsu";break; }
                        case 0b011: { instruction = "mulhu"; break; }
                        case 0b100: { instruction = "div";   break; }
                        case 0b101: { instruction = "divu";  break; }
                        case 0b110: { instruction = "rem";   break; }
                        case 0b111: { instruction = "remu";  break; }
                        default: continue;
                    }
                else continue;
                break;
            }
            case 0b00011: {
                // fence ?
                type = 'F';
                instruction = "fence";
                if (funct3 != 0b000) {
                    continue;
                }
                rs1 = (word << 4) >> 28;
                rs2 = (word << 8) >> 28;
                break;
            }
            case 0b11100: {
                type = 'E';
                instruction = (((word >> 20) & 0b1) ? "ebreak" : "ecall");
                break;
            }
        }

        out << "   " << left << setw(5) << setfill('0') << hex << (addr) << ":\t" <<
        setw(8) << right << word << "\t\t\t\t" << left << setw(7) << setfill(' ') << instruction ;
        if (type == 'R') {
            out << "\t" << x[rd] << ", " << x[rs1] << ", " << x[rs2];
        } else if (type == 'I') {
            out << "\t" << x[rd] << ", " << x[rs1] << ", " << dec << imm;
        } else if (type == 'S') {
            out << "\t" << x[rs2] << ", " << dec << imm << '(' << x[rs1] << ')';
        } else if (type == 'L') {
            out << "\t" << x[rd] << ", " << dec << imm << '(' << x[rs1] << ')';
        } else if (type == 'B') {
            out << "\t" << x[rs1] << ", " << x[rs2] << ", 0x" << hex << target << " <" << labels[target] << ">";
        } else if (type == 'U') {
            out << "\t" << x[rd] << ", " << dec << imm;
        } else if (type == 'J') {
            out << "\t" << x[rd] << ", 0x" << hex << target << " <" << labels[target] << ">";
        } else if (type == 'j') {
            out << "\t" << x[rd] << ", " << hex << imm << '(' << x[rs1] << ')';
        } else if (type == 'F') {
            out << "\t";
            string flags = "iorw";
            for (short fl = 0; fl < 4; fl++) if (rs1 & (1<<fl)) out << flags[fl];
            out << ", ";
            for (short fl = 0; fl < 4; fl++) if (rs2 & (1<<fl)) out << flags[fl];
        }
        out << endl;
    }

}

void printSymbols(ofstream& out, Symbol * symbolBuffer, Word symbolAmount, char * symbolStringBuffer) {
    if (!out.is_open()) {
        return throw exception();
    } 
    string types[] = {
        "NOTYPE", "OBJECT", "FUNC", "SECTION", "FILE", 
        "COMMON", "TLS", "", "", "", "LOOS", "", "HIOS", 
        "LOPROC", "", "HIPROC",
    };
    
    string binds[] = { 
        "LOCAL", "GLOBAL", "WEAK", "", "", "", "", "", "",
        "", "LOOS", "", "HIOS", "LOPROC", "", "HIPROC",
    };

    string vises[] = {
        "DEFAULT", "INTERNAL", "HIDDEN", "PROTECTED", 
        "EXPORTED", "SINGLETON", "ELIMINATE",
    };

    Half special_i[] = {
        0x0,    0xff00, 0xff00, 0xff00, 
        0xff01, 0xff02, 0xff1f, 0xff20, 
        0xff3f, 0xff3f, 0xff3f, 0xff3f, 
        0xfff1, 0xfff2, 0xffff, 0xffff,
    };

    string special_text[] = {
        "UNDEF",  "LORESERVE",     "LOPROC", "BEFORE", 
        "AFTER",  "AMD64_LCOMMON", "HIPROC", "LOOS", 
        "LOSUNW", "SUNW_IGNORE",   "HISUNW", "HIOS", 
        "ABS",    "COMMON",        "XINDEX", "HIRESERVE",
    };

    out << ".symtab" << endl <<
    "Symbol Value              Size Type     Bind     Vis       Index Name" << endl;
    for (Word i = 0; i < symbolAmount; i++) {
        short type = ((unsigned short)symbolBuffer[i].st_info) & 0xf;
        short bind = ((unsigned short)symbolBuffer[i].st_info) >> 4;
        short vis  = ((unsigned short)symbolBuffer[i].st_other) & 0x3;
        out << '[' << right << setw(4) << i << "] 0x" << 
        left << hex << setw(15) << symbolBuffer[i].st_value << ' ' <<
        right << dec << setw(5) << symbolBuffer[i].st_size << ' ' <<
        left << setw(8) << (type < 0x10 ? types[type] : "") << ' ' <<
        setw(8) << (bind < 0x10 ? binds[bind] : "") << ' ' <<
        setw(8) << (vis  < 0x7  ? vises[vis]  : "") << ' ';
        int j = 0;
        while (special_i[j] != symbolBuffer[i].st_shndx && j < 16) {
            j++;
        }
        if (j == 16) {
            out << right << setw(6) << symbolBuffer[i].st_shndx << ' ';
        } else {
            out << right << setw(6) << special_text[j] << ' ';
        }
        out << (symbolStringBuffer + symbolBuffer[i].st_name) << endl;
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        cerr << "Wrong ammount of arguments. Expected 2." << endl;
        return 1;
    }

    ifstream inputFile(argv[1], ios::in | ios::binary);
    if (!inputFile) {
        cerr << "Could not read file." << endl;
        return 1;
    }

    ElfHeader header;
    try { readHeader(inputFile, header); } catch (...) {
        cerr << "The file does not satisfy the requirements." << endl;
        return 1;
    }

    ProgramHeader * programHeader = new ProgramHeader[header.e_phnum];
    SectionHeader * sectionHeader = new SectionHeader[header.e_shnum];
    try { readProgramHeader(inputFile, header, programHeader);
            readSectionHeader(inputFile, header, sectionHeader); } catch (...) {
        cerr << "There was an error while reading headers." << endl;
        return 1;
    }
    delete[] programHeader;

    SectionHeader& namesSection = sectionHeader[header.e_shstrndx];
    char * stringBuffer = new char[namesSection.sh_size];
    try { getStringTable(inputFile, namesSection, stringBuffer); } catch (...) {
        cerr << "There was an error while reading header names." << endl;
        return 1;
    }

#ifdef NDEBUG
    printSections(header, stringBuffer, sectionHeader);
#endif

    // parse .strtab
    // getStringTable(inputFile, sectionHeader[6], new char[sectionHeader[6].sh_size], new Off[12]);

    // find .text and .symtab sections
    SectionHeader& text = sectionHeader[0], symtab = sectionHeader[0], strtab = sectionHeader[0];
    bool foundText = false, foundSymtab = false, foundStrtab = false;

    for (int i = 0; i < header.e_shnum; i++) {
        if (!foundText && sectionHeader[i].sh_type == 0x1 &&
            string(stringBuffer+sectionHeader[i].sh_name) == ".text") {
            text = sectionHeader[i];
            foundText = true;
        }
        if (!foundSymtab && sectionHeader[i].sh_type == 0x2 && 
            string(stringBuffer+sectionHeader[i].sh_name) == ".symtab") {
            symtab = sectionHeader[i];
            foundSymtab = true;
        }
        if (!foundStrtab && sectionHeader[i].sh_type == 0x3 &&
            string(stringBuffer+sectionHeader[i].sh_name) == ".strtab") {
            strtab = sectionHeader[i];
            foundStrtab = true;
        }
    }

    if (!foundSymtab || !foundText || !foundStrtab) {
        cerr << "No .symtab or .text or .strtab section." << endl;
        return 1;
    }

    char * symbolStringBuffer = new char[strtab.sh_size];
    try { getStringTable(inputFile, strtab, symbolStringBuffer); } catch (...) {
        cerr << "There was an error while reading header names." << endl;
        return 1;
    }
    
    Symbol * symbols = new Symbol[symtab.sh_size / symtab.sh_entsize];
    try { getSymbolTable(inputFile, symtab, symbols); } catch (...) {
        cerr << "There was an error while reading symbol table." << endl;
        return 1;
    }

    Word * wordBuffer = new Word[text.sh_size / sizeof(Word)];
    try { getProgBits(inputFile, text, wordBuffer); } catch (...) {
        cerr << "There was an error while reading program instructions." << endl;
        return 1;
    }
    inputFile.close();

    ofstream outputFile(argv[2]);
    if (!outputFile) {
        cerr << "Could not open file for writing." << endl;
        return 1;
    }

    unordered_map<Word, string> labels;
    getLabels(symbols, symtab.sh_size / symtab.sh_entsize, labels, symbolStringBuffer);

    printProgram(outputFile, wordBuffer, text.sh_size / sizeof(Word), text.sh_addr, stringBuffer + text.sh_name, labels);
    outputFile << endl;
    printSymbols(outputFile, symbols, symtab.sh_size / symtab.sh_entsize, symbolStringBuffer);
    outputFile.close();

    delete[] stringBuffer;
    delete[] sectionHeader;
    delete[] symbolStringBuffer;
    delete[] symbols;
    delete[] wordBuffer;
    return 0;
}
