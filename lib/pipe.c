#include <inc/lib.h>

#define debug 0

static int pipeclose(struct Fd *fd);
static ssize_t piperead(struct Fd *fd, void *buf, size_t n, off_t offset);
static int pipestat(struct Fd *fd, struct Stat *stat);
static ssize_t pipewrite(struct Fd *fd, const void *buf, size_t n, off_t offset);

struct Dev devpipe =
{
	.dev_id =	'p',
	.dev_name =	"pipe",
	.dev_read =	piperead,
	.dev_write =	pipewrite,
	.dev_close =	pipeclose,
	.dev_stat =	pipestat
};

#define PIPEBUFSIZ 32		// small to provoke races

struct Pipe {
	off_t p_rpos;		// read position
	off_t p_wpos;		// write position
	uint8_t p_buf[PIPEBUFSIZ];	// data buffer
};

int
pipe(int pfd[2])
{
	int r;
	struct Fd *fd0, *fd1;
	void *va;

	// allocate the file descriptor table entries
	if ((r = fd_alloc(&fd0)) < 0
	    || (r = sys_page_alloc(0, fd0, PTE_P|PTE_W|PTE_U|PTE_SHARE)) < 0)
		goto err;

	if ((r = fd_alloc(&fd1)) < 0
	    || (r = sys_page_alloc(0, fd1, PTE_P|PTE_W|PTE_U|PTE_SHARE)) < 0)
		goto err1;

	// allocate the pipe structure as first data page in both
	va = fd2data(fd0);
	if ((r = sys_page_alloc(0, va, PTE_P|PTE_W|PTE_U|PTE_SHARE)) < 0)
		goto err2;
	if ((r = sys_page_map(0, va, 0, fd2data(fd1), PTE_P|PTE_W|PTE_U|PTE_SHARE)) < 0)
		goto err3;

	// set up fd structures
	fd0->fd_dev_id = devpipe.dev_id;
	fd0->fd_omode = O_RDONLY;

	fd1->fd_dev_id = devpipe.dev_id;
	fd1->fd_omode = O_WRONLY;

	if (debug)
		cprintf("[%08x] pipecreate %08x\n", env->env_id, vpt[PGNUM(va)]);

	pfd[0] = fd2num(fd0);
	pfd[1] = fd2num(fd1);
	return 0;

    err3:
	sys_page_unmap(0, va);
    err2:
	sys_page_unmap(0, fd1);
    err1:
	sys_page_unmap(0, fd0);
    err:
	return r;
}

static int
_pipeisclosed(struct Fd *fd, struct Pipe *p)
{
	// Check pageref(fd) and pageref(p),
	// returning 1 if they're the same, 0 otherwise.
	// 
	// The logic here is that pageref(p) is the total
	// number of readers *and* writers, whereas pageref(fd)
	// is the number of file descriptors like fd (readers if fd is
	// a reader, writers if fd is a writer).
	// 
	// If the number of file descriptors like fd is equal
	// to the total number of readers and writers, then
	// everybody left is what fd is.  So the other end of
	// the pipe is closed.

	// LAB 5: Your code here.

	panic("_pipeisclosed not implemented");
	return 0;
}

int
pipeisclosed(int fdnum)
{
	struct Fd *fd;
	struct Pipe *p;
	int r;

	if ((r = fd_lookup(fdnum, &fd)) < 0)
		return r;
	p = (struct Pipe*) fd2data(fd);
	return _pipeisclosed(fd, p);
}

static ssize_t
piperead(struct Fd *fd, void *vbuf, size_t n, off_t offset)
{
	struct Pipe *p = (struct Pipe *) fd2data(fd);
	uint8_t *buf = (uint8_t *) vbuf;
	size_t i;

	for (i = 0; i < n; i++) {
		while (p->p_rpos == p->p_wpos) {
			// The pipe is currently empty.
			// If any data has been read, return it.
			// Otherwise, check for EOF; if not EOF, yield
			// and try again.
			if (i > 0)
				return i;
			else if (_pipeisclosed(fd, p))
				return 0;
			else
				sys_yield();
		}
		
		buf[i] = p->p_buf[p->p_rpos % PIPEBUFSIZ];
		// The increment must come AFTER we write to the buffer,
		// or the C compiler might update the pointer before writing
		// to the buffer!
		p->p_rpos++;
	}
	
	return i;
}

static ssize_t
pipewrite(struct Fd *fd, const void *vbuf, size_t n, off_t offset)
{
	// Write a loop that transfers one byte at a time.
	// Your code should be patterned on pipe_read above.
	// Unlike in read, it is not okay to write only some of the data:
	// if the pipe fills and you've only copied some of the data,
	// wait for the pipe to empty and then keep copying.
	// If the pipe is full and closed, return the number of characters
	// written.  Use _pipeisclosed to check whether the pipe is closed.

	// LAB 5: Your code here.

	panic("pipewrite not implemented");
	return -E_INVAL;
}

static int
pipestat(struct Fd *fd, struct Stat *stat)
{
	struct Pipe *p = (struct Pipe*) fd2data(fd);
	strcpy(stat->st_name, "<pipe>");
	stat->st_size = p->p_wpos - p->p_rpos;
	stat->st_isdir = 0;
	stat->st_dev = &devpipe;
	return 0;
}

static int
pipeclose(struct Fd *fd)
{
	(void) sys_page_unmap(0, fd);
	return sys_page_unmap(0, fd2data(fd));
}

