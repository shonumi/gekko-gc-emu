/*!
 * Copyright (C) 2005-2012 Gekko / ProjectCafe Emulator
 *
 * \file    loader.h
 * \author  Lightning, ShizZy <shizzy247@gmail.com>
 * \date    2012-02-12
 * \brief   Frontend interface for dvd/rom loading
 *
 * \section LICENSE
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * Official project repository can be found at:
 * http://code.google.com/p/gekko-gc-emu/
 */

#ifndef CORE_DVD_LOADER_H_
#define CORE_DVD_LOADER_H_

/// Frontend interface for DVD/ROM loading
namespace dvd {

/*! 
 * \brief Loads a bootable file (ROM/DVD image) to be ran by the emulator
 * \param filename Filename of ROM/DVD image to load
 * \return 0 on pass, non-zero error code on fail
 */
int LoadBootableFile(char* filename);

/*!
 * \brief Load a DOL (GameCube binary file)
 * \param filename Filename of DOL binary to load
 * \return 0 on pass, non-zero error code on fail
 */
int LoadDOL(char *filename);

/*!
 * \brief Load an ELF (executable and linkable format)
 * \param filename Filename of ELF binary to load
 * \return 0 on pass, non-zero error code on fail
 */
int LoadELF(char *filename);

/*!
 * \brief Load a GCM (GameCube DVD image, same as .ISO)
 * \param filename Filename of GCM binary to load
 * \return 0 on pass, non-zero error code on fail
 */
int LoadGCM(char *filename);

/*!
 * \brief Load a DMP (dump of GameCube DVD image)
 * \param filename Filename of ELF binary to load
 * \return 0 on pass, non-zero error code on fail
 */
// TODO (ShizZy): Deprecated - port gcm_dump to standard file IO, or remove from the project if 
//                  we no longer need it!
//int LoadDMP(char *filename); 

/*!
 * \brief Gets info for a GCM (GameCube DVD image)
 * \param filename Filename of GCM
 * \param filesize Filesize of GCM (reference)
 * \param BannerBuffer Pointer to GCM banner (reference)
 * \todo This should eventually be in its own GCM utilities library
 */
// TODO(ShizZy): Make cross platform 2012-03-07
int ReadGCMInfo(char *filename, unsigned long *filesize, void *BannerBuffer, void *Header);

extern char g_current_game_name[992];   ///< Currently loaded game name
extern char g_current_game_crc[7];      ///< Currently loaded game checksum

} // namespace

#endif // CORE_DVD_LOADER_H_