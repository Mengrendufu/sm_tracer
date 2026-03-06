/*****************************************************************************
 * Copyright (C) 2026 Sunny Matato
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar.
 * See http://www.wtfpl.net/ for more details.
 ****************************************************************************/
/*==========================================================================*/
#ifndef SNMTT_HLDC_PARSER_H_
#define SNMTT_HLDC_PARSER_H_

/*==========================================================================*/
/**
 * @brief
 */
void HLDCPaser_ctor(void);
/*..........................................................................*/
/**
 * @brief
 *
 * @param byte_
 */
void HLDCParser_trigger(uint8_t byte_);
/*..........................................................................*/
/**
 * @brief
 */
void HLDCParser_frmPost(unsigned char *frmOut);

#endif  /* SNMTT_HLDC_PARSER_H_ */
