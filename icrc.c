/*
 ** Fichero: icrc.c
 ** Autores:
 ** Alvaro Valiente Herrero 
 ** Diego Sanchez Moreno
 ** Usuario: i8112600
 */

typedef unsigned char uchar;
#define LOBYTE(x) ((uchar)((x) & 0xFF))
#define HIBYTE(x) ((uchar)((x) >> 8))

/*
**	icrc - Clacula un CRC de 16 bits para la matriz de caracteres "bufptr" de longitud "len"
**		con ciertos valores de "jinit" y "jrev"
**	Valores para jinit y jrev usados en el CRC de HDLC
**		jinit = 255 y jrev = -1
**		
**
*/

unsigned short icrc(unsigned short crc, unsigned char *bufptr,
	unsigned long len, short jinit, int jrev)
{
	unsigned short icrc1(unsigned short crc, unsigned char onech);
	static unsigned short icrctb[256],init=0;
	static uchar rchr[256];
	unsigned short j,cword=crc;
	static uchar it[16]={0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15};

	if (!init) {
		init=1;
		for (j=0;j<=255;j++) {
			icrctb[j]=icrc1(j << 8,(uchar)0);
			rchr[j]=(uchar)(it[j & 0xF] << 4 | it[j >> 4]);
		}
	}
	if (jinit >= 0) cword=((uchar) jinit) | (((uchar) jinit) << 8);
	else if (jrev < 0) cword=rchr[HIBYTE(cword)] | rchr[LOBYTE(cword)] << 8;
	for (j=1;j<=len;j++)
		cword=icrctb[(jrev < 0 ? rchr[bufptr[j]] :
			bufptr[j]) ^ HIBYTE(cword)] ^ LOBYTE(cword) << 8;
	return (jrev >= 0 ? cword : rchr[HIBYTE(cword)] | rchr[LOBYTE(cword)] << 8);
}
#undef LOBYTE
#undef HIBYTE
/* (C) Copr. 1986-92 Numerical Recipes Software . */
