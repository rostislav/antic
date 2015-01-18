/*=============================================================================

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

=============================================================================*/
/******************************************************************************

    Copyright (C) 2015 William Hart

******************************************************************************/

#include "nf_elem.h"

void nf_elem_fmpq_sub(nf_elem_t a, fmpq_t c, const nf_elem_t b, const nf_t nf)
{
   if (nf->flag & NF_LINEAR)
   {
      fmpz * den = LNF_ELEM_DENREF(a);
	  fmpz * num = LNF_ELEM_NUMREF(a);
	  
      _fmpq_sub(num, den, fmpq_numref(c), fmpq_denref(c), num, den);
   }
   else if (nf->flag & NF_QUADRATIC)
   {
      fmpz * den = QNF_ELEM_DENREF(a);
	  fmpz * num = QNF_ELEM_NUMREF(a);
	  slong len = 2;
	  
	  while (len != 0 && fmpz_is_zero(num + len - 1))
	     len--;
	  
      if (len == 0)
	  {
	     fmpz_set(num, fmpq_numref(c));
		 fmpz_set(den, fmpq_denref(c));
	  } else if (len == 1)
	     _fmpq_sub(num, den, fmpq_numref(c), fmpq_denref(c), num, den);
	  else
	  {
	     /* fast path */
		 if (fmpz_equal(fmpq_denref(c), den))
		    fmpz_sub(num, fmpq_numref(c), num);
		 else /* slow path */
		 {
		    fmpz_t d1, d2, g;
			
			fmpz_init(d1);
			fmpz_init(d2);
			fmpz_init(g);
			
			fmpz_gcd(g, fmpq_denref(c), den);
			fmpz_divexact(d1, fmpq_denref(c), g);
			fmpz_divexact(d2, den, g);
			
			fmpz_mul(num + 1, num + 1, d1);
			fmpz_mul(num, num, d1);
			fmpz_mul(den, den, d1);
			
			fmpz_submul(num, d2, fmpq_numref(c));
		    fmpz_neg(num, num);
			
			fmpz_clear(g);
			fmpz_clear(d1);
			fmpz_clear(d2);
		 }
		 
		 _fmpq_poly_canonicalise(num, den, 2);
	  }
   } else
   {
      fmpq_poly_fmpq_sub(NF_ELEM(a), c, NF_ELEM(a));
   }
}
