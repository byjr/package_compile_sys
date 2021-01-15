#include "../slog/slog.h"
#include "dll_fifo.h"
#include <stdlib.h>
dll_fifo_t *dll_fifo_create ( dll_fifo_par_t *par ) {
	dll_fifo_t *ptr = ( dll_fifo_t * ) calloc ( 1, sizeof ( dll_fifo_t ) );
	if ( !ptr ) {
		s_err ( "(dll_fifo_t*)calloc" );
		return NULL;
	}
	if ( par ) {
		size_t i = 0;
		void *data =  NULL;
		bool res = false;
		ptr->mPar = par;
		if ( par->dat_create ) {
			for ( i = 0; i < par->size; i++ ) {
				data = par->dat_create ( par->dat_par );
				if ( !data ) {
					s_err ( "dat_create data[%u] failed!!", i );
					dll_fifo_destroy ( ptr );
					return NULL;
				}
				res = dll_fifo_push_back ( ptr, data );
				if ( !res ) {
					s_err ( "dll_fifo_push_back data[%u] failed!!", i );
					par->dat_destroy ( ptr->mPar->dat_par, data );
					dll_fifo_destroy ( ptr );
					return NULL;
				}
			}
		}
	}
	return ptr;
}
bool dll_fifo_push_back ( dll_fifo_t *ptr, void *data ) {
	if ( !ptr ) return false;
	dll_node_t *nd = ( dll_node_t * ) calloc ( 1, sizeof ( dll_node_t ) );
	if ( !nd ) {
		s_err ( "(dll_node_t* )calloc" );
		return false;
	}
	nd->data = data;
	if ( ptr->size == 0 ) {
		ptr->front = ptr->back = nd;
		ptr->size ++;
		return true;
	}
	nd->prev = ptr->back;
	ptr->back->next = nd;
	ptr->back = nd;
	ptr->size ++;
	return true;
}
bool dll_fifo_push_front ( dll_fifo_t *ptr, void *data ) {
	if ( !ptr ) return false;
	dll_node_t *nd = ( dll_node_t * ) calloc ( 1, sizeof ( dll_node_t ) );
	if ( !nd ) {
		s_err ( "(dll_node_t* )calloc" );
		return false;
	}
	nd->data = data;
	if ( ptr->size == 0 ) {
		ptr->front = ptr->back = nd;
		ptr->size ++;
		return true;
	}
	nd->next = ptr->front;
	ptr->front->prev = nd;
	ptr->front = nd;
	ptr->size ++;
	return true;
}
void *dll_fifo_pop_front ( dll_fifo_t *ptr ) {
	if ( !ptr ) return NULL;
	if ( ptr->size == 0 ) {
		return NULL;
	}
	dll_node_t *nd = NULL;
	void *data = NULL;
	if ( ptr->size == 1 ) {
		nd = ptr->front;
		ptr->front = NULL;
		ptr->size --;
	} else {
		nd = ptr->front;
		ptr->front->next->prev = NULL;
		ptr->front = ptr->front->next;
		ptr->size --;
	}
	data = nd->data;
	free ( nd );
	return data;
}
void *dll_fifo_pop_back ( dll_fifo_t *ptr ) {
	if ( !ptr ) return NULL;
	if ( ptr->size == 0 ) {
		return NULL;
	}
	dll_node_t *nd = NULL;
	void *data = NULL;
	if ( ptr->size == 1 ) {
		nd = ptr->back;
		ptr->back = NULL;
		ptr->size --;
	} else {
		nd = ptr->back;
		ptr->back->prev->next = NULL;
		ptr->back = ptr->back->prev;
		ptr->size --;
	}
	data = nd->data;
	free ( nd );
	return data;
}
bool dll_fifo_insert ( dll_fifo_t *ptr, void *data, size_t n ) {
	if ( !ptr ) return false;
	if ( n == 0 ) {
		return dll_fifo_push_front ( ptr, data );
	}
	if ( n >= ptr->size ) {
		return dll_fifo_push_back ( ptr, data );
	}
	size_t i = 0;
	dll_node_t *idx = ptr->front;
	for ( ; idx; idx = idx->next, i++ ) {
		if ( i == n ) {
			break;
		}
	}
	dll_node_t *nd = ( dll_node_t * ) calloc ( 1, sizeof ( dll_node_t ) );
	if ( !nd ) {
		s_err ( "(dll_node_t* )calloc" );
		return false;
	}
	nd->data = data;
	nd->next = idx;
	nd->prev = idx->prev;
	idx->prev->next = nd;
	idx->prev = nd;
	ptr->size ++;
	return true;
}
bool dll_fifo_erase ( dll_fifo_t *ptr, void *data ) {
	if ( !ptr ) return false;
	dll_node_t *idx = ptr->front;
	for ( ; idx; idx = idx->next ) {
		if ( idx->data == data ) {
			break;
		}
	}
	if ( ! ( idx && ( idx->data == data ) ) ) {
		s_err ( "can't find node: %p", data );
		return false;
	}
	if ( idx->prev ) {
		idx->prev->next = idx->next;
	} else {
		ptr->front = idx->next;
	}
	if ( idx->next ) {
		idx->next->prev = idx->prev;
	} else {
		ptr->back = idx->prev;
	}
	if ( ptr->mPar->dat_destroy ) {
		ptr->mPar->dat_destroy ( ptr->mPar->dat_par, idx->data );
	}
	free ( idx );
	ptr->size --;
	return true;
}
bool dll_fifo_remove ( dll_fifo_t *ptr, size_t n ) {
	if ( !ptr ) return false;
	size_t i = 0;
	dll_node_t *idx = ptr->front;
	for ( ; idx; idx = idx->next, i++ ) {
		if ( i == n ) {
			break;
		}
	}
	if ( ! ( idx && ( i == n ) ) ) {
		s_err ( "can't find node[%u]", n );
		return -1;
	}
	if ( idx->prev ) {
		idx->prev->next = idx->next;
	} else {
		ptr->front = idx->next;
	}
	if ( idx->next ) {
		idx->next->prev = idx->prev;
	} else {
		ptr->back = idx->prev;
	}
	if ( ptr->mPar->dat_destroy ) {
		ptr->mPar->dat_destroy ( ptr->mPar->dat_par, idx->data );
	}
	free ( idx );
	ptr->size --;
	return true;
}
void *dll_fifo_query ( dll_fifo_t *ptr, ssize_t n ) {
	if ( !ptr ) return NULL;
	size_t i = 0;
	dll_node_t *nd = ptr->front;
	if ( n < 0 ) {
		for ( ; nd; nd = nd->next, i++ ) {
			if ( ptr->mPar->dat_show ) {
				ptr->mPar->dat_show ( ptr->mPar->dat_par, nd->data );
			} else {
				s_inf ( "nd[%u]:data=%p", i, nd->data );
			}
		}
		return NULL;
	}
	for ( ; nd; nd = nd->next ) {
		if ( i == ( size_t ) n ) {
			if ( ptr->mPar->dat_show ) {
				ptr->mPar->dat_show ( ptr->mPar->dat_par, nd->data );
			} else {
				s_inf ( "dll_fifo[%u]=%p", i, nd->data );
			}
			return nd->data;
		}
		i++;
	}
	s_err ( "node[%d]:dosen't exist!", n );
	return NULL;
}
void *dll_fifo_get ( dll_fifo_t *ptr, size_t n ) {
	if ( !ptr ) return NULL;
	size_t i = 0;
	dll_node_t *nd = ptr->front;
	for ( ; nd; nd = nd->next, i++ ) {
		if ( i == n ) {
			return nd->data;
		}
	}
	return NULL;
}
void *dll_fifo_rquery ( dll_fifo_t *ptr, ssize_t n ) {
	if ( !ptr ) return NULL;
	size_t i = 0;
	if ( n < 0 ) {
		dll_node_t *nd = ptr->back;
		for ( ; nd; nd = nd->prev, i++ ) {
			if ( ptr->mPar->dat_show ) {
				ptr->mPar->dat_show ( ptr->mPar->dat_par, nd->data );
			} else {
				s_inf ( "nd[%u]:data=%p", i, nd->data );
			}
		}
		return NULL;
	}
	dll_node_t *nd = ptr->back;
	for ( ; nd; nd = nd->prev, i++ ) {
		if ( i == n ) {
			if ( ptr->mPar->dat_show ) {
				ptr->mPar->dat_show ( ptr->mPar->dat_par, nd->data );
			} else {
				s_inf ( "dll_fifo[%u]=%p", i, nd->data );
			}
			return nd->data;
		}
	}
	s_err ( "node[%d]:dosen't exist!", n );
	return NULL;
}
bool dll_fifo_clear ( dll_fifo_t *ptr ) {
	if ( !ptr ) return false;
	dll_node_t *nd = ptr->front;
	dll_node_t *tmp;
	for ( ; nd; ) {
		tmp = nd;
		nd = nd->next;
		if ( ptr->mPar->dat_destroy ) {
			ptr->mPar->dat_destroy ( ptr->mPar->dat_par, tmp->data );
		}
		free ( tmp );
	}
	ptr->front = ptr->back = NULL;
	ptr->size = 0;
	return true;
}
bool dll_fifo_destroy ( dll_fifo_t *ptr ) {
	if ( !ptr ) return false;
	dll_node_t *nd = ptr->front;
	dll_node_t *tmp;
	for ( ; nd; ) {
		tmp = nd;
		nd = nd->next;
		if ( ptr->mPar->dat_destroy ) {
			ptr->mPar->dat_destroy ( ptr->mPar->dat_par, tmp->data );
		}
		free ( tmp );
	}
	free ( ptr );
	return true;
}
