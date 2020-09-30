//--------------------autom.c start-----------------------------
#define _GNU_SOURCE
#include <string.h>
#include "autom.h"
#include "../slog/slog.h"
#include "../common/misc.h"
autom_t *autom_create ( autom_par_t *par ) {
	autom_t *ptr = ( autom_t * ) calloc ( 1, sizeof ( autom_t ) );
	if ( !ptr ) {
		s_err ( "out of memery!" );
		return NULL;
	}
	ptr->mPar = par;
	if ( ptr->mPar->mode == AUTOM_MODE_MIN ) {
		ptr->mPar->mode = AUTOM_MODE_MALLOC;
	}
	if ( ptr->mPar->capacity == 0 ) {
		ptr->mPar->capacity = 16 * 1024 * 4 / 3 + 1024;
	}
	if ( ptr->mPar->mode == AUTOM_MODE_ALLOC ) {
		ptr->head = ptr->mPar->ex_tack;
	} else if ( ptr->mPar->mode == AUTOM_MODE_MALLOC ) {
		ptr->head = ( char * ) malloc ( ptr->mPar->capacity );
		if ( !ptr->head ) {
			s_err ( "autom_create/malloc oom" );
			free ( ptr );
			return NULL;
		}
	} else {
		s_err ( "unsurport mode!!" );
		free ( ptr );
		return NULL;
	}
	ptr->tail = ptr->head;
	if ( ptr->mPar->aftc ) {
		if ( ptr->mPar->aftc ( ptr ) < 0 ) {
			s_err ( "%s/aftc failed!!", __func__ );
			return NULL;
		}
	}
	return ptr;
}
char *autom_write_query ( autom_t *ptr, size_t bytes ) {
	size_t need_spece_bytes = ptr->content + bytes + 1;
	if ( need_spece_bytes > ptr->mPar->capacity ) {
		if ( ptr->mPar->mode == AUTOM_MODE_ALLOC ) {
			return NULL;
		}
		ptr->mPar->capacity = need_spece_bytes;
		ptr->head = ( char * ) realloc ( ptr->head, need_spece_bytes );
		if ( !ptr->head ) {
			s_err ( "autom_capacity_prepare/realloc!!" );
			return NULL;
		}
	}
	return ptr->head + ptr->content;
}
char *autom_write_get ( autom_t *ptr, size_t *pBytes ) {
	ssize_t idel_bytes = ptr->head + ptr->mPar->capacity - ptr->tail - 1;
	if ( idel_bytes <= 0 ) {
		return NULL;
	}
	if ( pBytes ) {
		*pBytes = idel_bytes;
	}
	return ptr->tail;
}
int autom_write_sync ( autom_t *ptr, size_t bytes ) {
	ptr->head[ ptr->content + bytes ] = 0;
	ptr->content += bytes;
	ptr->tail = ptr->head + ptr->content;
	if ( ptr->mPar->aftw ) {
		if ( ptr->mPar->aftw ( ptr ) < 0 ) {
			s_err ( "%s/dfq_fifo_tdWrite failed!!", __func__ );
			return -1;
		}
	}
	return 0;
}
int autom_write ( autom_t *ptr, const char *buf, size_t bytes ) {
	char *wi = autom_write_query ( ptr, bytes );
	if ( !wi ) {
		return -1;
	}
	memcpy ( wi, buf, bytes );
	autom_write_sync ( ptr, bytes );
	return 0;
}
int autom_write_syncn ( autom_t *ptr, size_t bytes ) {
	ptr->content += bytes;
	ptr->tail = ptr->head + ptr->content;
	ptr->tail[0] = 0;
	return 0;
}
int autom_writen ( autom_t *ptr, const char *buf, size_t bytes ) {
	char *wi = autom_write_query ( ptr, bytes );
	if ( !wi ) {
		return -1;
	}
	memcpy ( wi, buf, bytes );
	autom_write_syncn ( ptr, bytes );
	return 0;
}
char *autom_read_query ( autom_t *ptr, size_t bytes ) {
	if ( ptr->content < bytes ) {
		return NULL;
	}
	return ptr->tail - ptr->content;
}
char *autom_read_get ( autom_t *ptr, size_t *pBytes ) {
	if ( ptr->content < 1 ) {
		return NULL;
	}
	if ( pBytes ) {
		*pBytes = ptr->content;
	}
	return ptr->tail - ptr->content;
}
void autom_read_sync ( autom_t *ptr, size_t bytes ) {
	ptr->content -= bytes;
}
ssize_t autom_read ( autom_t *ptr, char *buf, size_t bytes ) {
	char *ri = autom_read_query ( ptr, bytes );
	if ( !ri ) {
		return -1;
	}
	memcpy ( buf, ri, bytes );
	autom_read_sync ( ptr, bytes );
	return 0;
}
int autom_reset ( autom_t *ptr ) {
	ptr->content = 0;
	ptr->tail = ptr->head;
	return 0;
}
int autom_clear ( autom_t *ptr ) {
	memset ( ptr->head, 0, ptr->content );
	return autom_reset ( ptr );
}
int autom_destroy ( autom_t *ptr ) {
	if ( !ptr ) return -1;
	// if(ptr->mode == AUTOM_MODE_ALLOC){

	// }
	if ( ptr->mPar->mode == AUTOM_MODE_MALLOC ) {
		if ( ptr->head ) {
			free ( ptr->head );
		}
	}
	free ( ptr );
	return 0;
}
ssize_t autom_find_first_ch ( autom_t *ptr, size_t pos, char ch ) {
	if ( !ptr ) {
		s_err ( "ptr is null" );
		return -1;
	}
	if ( ptr->tail - ptr->content >  ptr->head ) {
		s_err ( "Content part has been read out,the handler is not implemented!!" );
		return -1;
	}
	if ( ptr->content - pos < 1 ) {
		return -1;
	}
	char *idx = memchr ( ptr->head + pos, ch, ptr->content );
	if ( !idx ) {
		return -1;
	}
	return idx - ptr->head;
}
ssize_t autom_find_first_sub ( autom_t *ptr, size_t pos, const char *sub, size_t subb ) {
	if ( ! ( ptr && sub ) ) {
		s_err ( "ptr is null" );
		return -1;
	}
	if ( ptr->tail - ptr->content >  ptr->head ) {
		s_err ( "Content part has been read out,the handler is not implemented!!" );
		return -1;
	}
	if ( ptr->content - pos < subb ) {
		return -1;
	}
	char *idx = memmem ( ptr->head + pos, ptr->content, sub, subb );
	if ( !idx ) {
		return -1;
	}
	return idx - ptr->head;
}
int autom_subctt ( autom_t *ptr, size_t pos, size_t bytes ) {
	if ( !ptr ) {
		s_err ( "ptr is null" );
		return -1;
	}
	if ( ptr->tail - ptr->content >  ptr->head ) {
		s_err ( "Content part has been read out,the handler is not implemented!!" );
		return -1;
	}
	char *valid = ptr->head + pos;
	if ( !bytes || ( valid + bytes > ptr->tail ) ) {
		bytes = ptr->tail - valid;
	}
	if ( pos > 0 ) {
		memcpy ( ptr->head, valid, bytes );
	}
	ptr->content = bytes;
	ptr->tail = ptr->head + bytes;
	ptr->tail[0] = 0;
	return 0;
}
//--------------------autom.c end------------------------------
