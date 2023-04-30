#ifndef basicElfWrapper_hpp
#define basicElfWrapper_hpp

#include <vector>
#include <string>
#include "elfio/elfio.hpp"

using namespace std;
using namespace ELFIO;

void wrapElf(vector<unsigned char> rawBinary, unsigned long long offset, string file){
//vector<unsigned char> wrapElf(vector<unsigned char> rawBinary, string file){
    elfio writer;

    // You can't proceed without this function call!
    writer.create( ELFCLASS64, ELFDATA2LSB );

    writer.set_os_abi( ELFOSABI_LINUX );
    writer.set_type( ET_EXEC );
    writer.set_machine( 183 );

    // Create code section
    section* text_sec = writer.sections.add( ".text" );
    text_sec->set_type( SHT_PROGBITS );
    text_sec->set_flags( SHF_ALLOC | SHF_EXECINSTR | SHF_WRITE );
    text_sec->set_addr_align( 0x10 );

    auto text = (char*)rawBinary.data();
    text_sec->set_data( text, rawBinary.size() );

    // Create a loadable segment
    segment* text_seg = writer.segments.add();
    text_seg->set_type( PT_LOAD );
    text_seg->set_virtual_address( offset );
    text_seg->set_physical_address( offset );
    text_seg->set_flags( PF_X | PF_R | PF_W );
    text_seg->set_align( 0x10000 );

    // Add code section into program segment
    text_seg->add_section_index( text_sec->get_index(), text_sec->get_addr_align() );

    // // Create data section*
    // section* data_sec = writer.sections.add( ".data" );
    // data_sec->set_type( SHT_PROGBITS );
    // data_sec->set_flags( SHF_ALLOC | SHF_WRITE );
    // data_sec->set_addr_align( 0x4 );
    //
    // char data[] = { '\x48', '\x65', '\x6C', '\x6C', '\x6F',   // msg: db   'Hello, World!', 10
    //                 '\x2C', '\x20', '\x57', '\x6F', '\x72',
    //                 '\x6C', '\x64', '\x21', '\x0A'
    //               };
    // data_sec->set_data( data, sizeof( data ) );
    //
    // // Create a read/write segment
    // segment* data_seg = writer.segments.add();
    // data_seg->set_type( PT_LOAD );
    // data_seg->set_virtual_address( 0x08048020 );
    // data_seg->set_physical_address( 0x08048020 );
    // data_seg->set_flags( PF_W | PF_R );
    // data_seg->set_align( 0x10 );
    //
    // // Add code section into program segment
    // data_seg->add_section_index( data_sec->get_index(), data_sec->get_addr_align() );

//     section* note_sec = writer.sections.add( ".note" );
//     note_sec->set_type( SHT_NOTE );
//     note_sec->set_addr_align( 1 );
//     note_section_accessor note_writer( writer, note_sec );
//     note_writer.add_note( 0x01, "Created by ELFIO", 0, 0 );
//     char descr[6] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36};
//     note_writer.add_note( 0x01, "Never easier!", descr, sizeof( descr ) );

    // Setup entry point
    writer.set_entry( offset );

    // Create ELF file
    writer.save( file );
    // stringstream ss;
    // writer.save(ss);
    // return
}

#endif
