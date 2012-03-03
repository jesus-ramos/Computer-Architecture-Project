#ifndef DM_CACHE_H
#define DM_CACHE_H

#define DMC_DEBUG 1

#define DM_MSG_PREFIX "cache"
#define DMC_PREFIX "dm-cache: "

#if DMC_DEBUG
#define DPRINTK( s, arg... ) printk(DMC_PREFIX s "\n", ##arg)
#define VALSECTOR( w,x,y,z) validate_sector(w,x,y,z)
#define VALFETCH( x, y, z  ) validate_fetch(x,y,z)
#else
#define DPRINTK( s, arg... )
#define VALSECTOR( w,x,y,z) 
#define VALFETCH(x,y,z) 
#endif

/* Default cache parameters */
#define DEFAULT_CACHE_SIZE	65536
#define DEFAULT_CACHE_ASSOC	1024
#define DEFAULT_BLOCK_SIZE	8
#define CONSECUTIVE_BLOCKS	512

/* Write policy */
#define WRITE_THROUGH 0
#define WRITE_BACK 1
#define DEFAULT_WRITE_POLICY WRITE_THROUGH

/* Number of pages for I/O */
#define DMCACHE_COPY_PAGES 1024

/* States of a cache block */
#define INVALID		0
#define VALID		1	/* Valid */
#define RESERVED	2	/* Allocated but data not in place yet */
#define DIRTY		4	/* Locally modified */
#define WRITEBACK	8	/* In the process of write back */
#define WRITETHROUGH	16	/* In the process of write back */
#define READY           0
#define PENDING_WRITE   1
#define PENDING_READ    2

#define HASH 0         /* Use hash_long */
#define UNIFORM 1      /* Evenly distributed */
#define DEFAULT_HASHFUNC UNIFORM


#define is_state(x, y)		(x & y)
#define set_state(x, y)		(x |= y)
#define clear_state(x, y)	(x &= ~y)

/*
* Validations
*/
#define SEG_SIZE_ORDER  0 
#define SEG_SIZE_BYTES  512
/*
* Virtual Cache Mapping
*/
#define MAX_SRC_DEVICES		8

int	dm_dev_identifier = 0;

struct v_map {
	struct dm_dev *src_dev;
	dev_t vcache_dev;

	sector_t dev_size;
	sector_t dev_offset;

	struct dm_target *ti;
} virtual_mapping[MAX_SRC_DEVICES];

int	init_flag = 0;
int	init_cache_structure = 0;
//struct 	dm_dev *dev_arr[MAX_SRC_DEVICES];  

/*
 * Cache context
 */
struct cache_c {
	struct dm_dev *src_dev;		/* Source devices*/

	struct dm_dev *cache_dev;	/* Cache device */
	struct dm_kcopyd_client *kcp_client; /* Kcopyd client for writing back data */

	struct cacheblock *cache;	/* Hash table for cache blocks */
	sector_t size;			/* Cache size */
	unsigned int bits;		/* Cache size in bits */
	unsigned int assoc;		/* Cache associativity */
	unsigned int block_size;	/* Cache block size */
	unsigned int block_shift;	/* Cache block size in bits */
	unsigned int block_mask;	/* Cache block mask */
	unsigned int consecutive_shift;	/* Consecutive blocks size in bits */
	unsigned long counter;		/* Logical timestamp of last access */
	unsigned int write_policy;	/* Cache write policy */
	sector_t dirty_blocks;		/* Number of dirty blocks */

	spinlock_t lock;		/* Lock to protect page allocation/deallocation */
	struct page_list *pages;	/* Pages for I/O */
	unsigned int nr_pages;		/* Number of pages */
	unsigned int nr_free_pages;	/* Number of free pages */
	wait_queue_head_t destroyq;	/* Wait queue for I/O completion */
	atomic_t nr_jobs;		/* Number of I/O jobs */
	struct dm_io_client *io_client;   /* Client memory pool*/

	/* Stats */
	unsigned long reads;		/* Number of reads */
	unsigned long writes;		/* Number of writes */
	unsigned long cache_hits;	/* Number of cache hits */
	unsigned long replace;		/* Number of cache replacements */
	unsigned long writeback;	/* Number of replaced dirty blocks */
	unsigned long dirty;		/* Number of submitted dirty blocks */
}shared_cache;

/* Cache block metadata structure */
struct cacheblock {
	spinlock_t lock;	/* Lock to protect operations on the bio list */
	sector_t block;		/* Sector number of the cached block */
	unsigned short state;	/* State of a block */
	atomic_t status; 
	unsigned long counter;	/* Logical timestamp of the block's last access */
	struct bio_list bios;	/* List of pending bios */
};

/* Structure for a kcached job */
struct kcached_job {
	struct list_head list;
	struct cache_c *dmc;
	struct bio *bio;	/* Original bio */
	struct dm_io_region src;
	struct dm_io_region dest;
	struct cacheblock *cacheblock;
	int rw;
	int hit;
	int vdisk;
	/*
	 * When the original bio is not aligned with cache blocks,
	 * we need extra bvecs and pages for padding.
	 */
	struct bio_vec *bvec;
	unsigned int nr_pages;
	struct page_list *pages;
};

static int cache_insert(struct cache_c *dmc, sector_t block,sector_t cache_block); 
//static int cache_read_hit(struct cache_c *dmc, struct bio* bio,sector_t cache_block);
//static int cache_write_cache(struct cache_c *dmc, struct bio* bio, sector_t cache_block,
//                                int hit, int writethrough);
static int validate_sector(sector_t sector_source, sector_t sector_cache,struct cache_c *dmc,int vdisk);
static int validate_fetch(sector_t sector_source, struct page *fetch_page,int vdisk);

static void io_callback(unsigned long error, void *context);
static int do_io(struct kcached_job *job);
static int do_complete(struct kcached_job *job);
static int do_bio_read(struct block_device *bi_bdev, sector_t block,struct page *page_read);
static int virtual_cache_map(struct bio *bio);

#endif /* DM_CACHE_H */
