/*============================================================================

    This file is part of FLINT.

    FLINT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    FLINT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FLINT; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

===============================================================================*/
/****************************************************************************

   Copyright (C) 2010 William Hart
   
*****************************************************************************/

#include <stdlib.h>
#include <mpir.h>
#include "flint.h"
#include "fmpz.h"
#include "fmpz_vec.h"
#include "fmpz_poly.h"

void _fmpz_poly_mulhigh_kara_recursive(fmpz * out, const fmpz * pol1, const fmpz * pol2, fmpz * temp, ulong length);

/**
    Multiplication using truncated karatsuba. Below length 7, classical truncated multiplication
    is always theoretically faster, so we switch to that as the basecase.

    Above that we use the ordinary (left/right) karatsuba identity and recursively do one full
    karatsuba multiplication and two truncated karatsuba multiplications.
*/

// recursive Karatsuba mulhigh
void _fmpz_poly_mulhigh_kara_recursive(fmpz * out, const fmpz * pol1, const fmpz * pol2, fmpz * temp, ulong length)
{
   ulong m1 = length/2;
   ulong m2 = length - m1;
   int odd = (length & 1);
   
   if (length <= 6)
   {
      _fmpz_poly_mulhigh_classical(out, pol1, length, pol2, length, length - 1);
      return;
   }

   _fmpz_vec_add(out, pol1, pol1 + m1, m1);
   if (odd) fmpz_set(out + m1, pol1 + 2*m1);
   
   _fmpz_vec_add(out + m2, pol2, pol2 + m1, m1);
   if (odd) fmpz_set(out + m2 + m1, pol2 + 2*m1);
   
   _fmpz_poly_mulhigh_kara_recursive(temp, out, out + m2, temp + 2*m2, m2);
   
   _fmpz_poly_mul_karatsuba(out + 2*m1, pol1 + m1, m2, pol2 + m1, m2);
   fmpz_zero(out + 2*m1 - 1);
   
   _fmpz_poly_mulhigh_kara_recursive(out, pol1, pol2, temp + 2*m2, m1);
   
   _fmpz_vec_sub(temp + m2 - 1, temp + m2 - 1, out + m2 - 1, 2*m1 - m2); 
   _fmpz_vec_sub(temp + m2 - 1, temp + m2 - 1, out + 2*m1 + m2 - 1, m2); 
   
   _fmpz_vec_add(out + length - 1, out + length - 1, temp + m2 - 1, m2); 
   _fmpz_vec_zero(out, length - 1);
}

// Assumes poly1 and poly2 are not length 0
void _fmpz_poly_mulhigh_karatsuba_n(fmpz * res, const fmpz * poly1, const fmpz * poly2, ulong len)
{
   fmpz * temp;
   ulong loglen = 0, length;

   if (length == 1)
   {
      fmpz_mul(res, poly1, poly2);
      return;
   }

   while ((1L<<loglen) < len) loglen++;
   length = (1L<<loglen);

   temp = _fmpz_vec_init(2*length); 
   
   _fmpz_poly_mulhigh_kara_recursive(res, poly1, poly2, temp, len);  

   _fmpz_vec_clear(temp, 2*length);
}

void fmpz_poly_mulhigh_karatsuba_n(fmpz_poly_t res, 
                         const fmpz_poly_t poly1, const fmpz_poly_t poly2, ulong length)
{
   ulong i;
   int clear1 = 0, clear2 = 0;
   
   fmpz * pol1, * pol2;

   if ((poly1->length == 0) || (poly2->length == 0))  
   {
      fmpz_poly_zero(res);
      return;
   }

   if (poly1->length != length)
   {
      pol1 = (fmpz *) calloc(length, sizeof(fmpz));
      for (i = 0; i < poly1->length; i++)
         pol1[i] = poly1->coeffs[i];
      clear1 = 1;
   } else 
      pol1 = poly1->coeffs;

   if (poly2->length != length)
   {
      pol2 = (fmpz *) calloc(length, sizeof(fmpz));
      for (i = 0; i < poly2->length; i++)
         pol2[i] = poly2->coeffs[i];
      clear2 = 1;
   } else 
      pol2 = poly2->coeffs;

   if (res != poly1 && res != poly2)
   {
      fmpz_poly_fit_length(res, 2*length - 1);

     _fmpz_poly_mulhigh_karatsuba_n(res->coeffs, pol1, pol2, length);
     _fmpz_poly_set_length(res, 2*length - 1);
     _fmpz_poly_normalise(res);
   } else
   {
      fmpz_poly_t temp;
      fmpz_poly_init(temp);
      fmpz_poly_fit_length(temp, 2*length - 1);

      _fmpz_poly_mulhigh_karatsuba_n(temp->coeffs, pol1, pol2, length);
      _fmpz_poly_set_length(temp, 2*length - 1);
      _fmpz_poly_normalise(temp);

      fmpz_poly_swap(temp, res);
      fmpz_poly_clear(temp);
   }

   if (clear1) free(pol1);
   if (clear2) free(pol2);
}