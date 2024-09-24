/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Based on arch/arm/include/asm/pgalloc.h
 *
 * Copyright (C) 2000-2001 Russell King
 * Copyright (C) 2012 ARM Ltd.
 */
#ifndef __ASM_PGALLOC_H
#define __ASM_PGALLOC_H

#include <asm/pgtable-hwdef.h>
#include <asm/processor.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

#include <asm-generic/pgalloc.h>	/* for pte_{alloc,free}_one */

#define PGD_SIZE	(PTRS_PER_PGD * sizeof(pgd_t))

#if CONFIG_PGTABLE_LEVELS > 2

static inline pmd_t *pmd_alloc_one(struct mm_struct *mm, unsigned long addr)
{
	gfp_t gfp = GFP_PGTABLE_USER;
	struct page *page = NULL;

	if (mm == &init_mm)
		gfp = GFP_PGTABLE_KERNEL;
	//Gol
	//test my hook
	if(alloc_pvtpool_pgtble)// check for mm->profile inside the kernel module
	  {
	        printk("inside the alloc_pvtpool_pgtble\n");
		//call our hook alloc_pvtpool_pgtble
		page = alloc_pvtpool_pgtble(mm);
	}
	if (!page)
	{
	  //printk("if we are here, it means page was empty\n");
		page = alloc_page(gfp);
	}
	if (!page)
		return NULL;
	
	if (!pgtable_pmd_page_ctor(page)) {
		__free_page(page);
		return NULL;
	}
	return page_address(page);
}

static inline void pmd_free(struct mm_struct *mm, pmd_t *pmdp)
{
	BUG_ON((unsigned long)pmdp & (PAGE_SIZE-1));
	pgtable_pmd_page_dtor(virt_to_page(pmdp));
	//Gol
	//test my hook
	if (free_pvtpool_pgtble)
	  {
	    if(free_pvtpool_pgtble((unsigned long)pmdp) == 0)
	      return;
	  }
	free_page((unsigned long)pmdp);
}

static inline void __pud_populate(pud_t *pudp, phys_addr_t pmdp, pudval_t prot)
{
	set_pud(pudp, __pud(__phys_to_pud_val(pmdp) | prot));
}

static inline void pud_populate(struct mm_struct *mm, pud_t *pudp, pmd_t *pmdp)
{
	__pud_populate(pudp, __pa(pmdp), PMD_TYPE_TABLE);
}
#else
static inline void __pud_populate(pud_t *pudp, phys_addr_t pmdp, pudval_t prot)
{
	BUILD_BUG();
}
#endif	/* CONFIG_PGTABLE_LEVELS > 2 */

#if CONFIG_PGTABLE_LEVELS > 3

static inline pud_t *pud_alloc_one(struct mm_struct *mm, unsigned long addr)
{
  //  if(alloc_pvtpool_pgtble)
  //return (pud_t *)alloc_pvtpool_pgtble(mm);
  //else
  return (pud_t *)__get_free_page(GFP_PGTABLE_USER);
}

static inline void pud_free(struct mm_struct *mm, pud_t *pudp)
{
	BUG_ON((unsigned long)pudp & (PAGE_SIZE-1));
	//	if(free_pvtpool_pgtble)
	//{
	//  if(free__pvtpool_pgtble((unsigned long)pudp) == 0)
	//    return;
	//}
	free_page((unsigned long)pudp);
}

static inline void __pgd_populate(pgd_t *pgdp, phys_addr_t pudp, pgdval_t prot)
{
	set_pgd(pgdp, __pgd(__phys_to_pgd_val(pudp) | prot));
}

static inline void pgd_populate(struct mm_struct *mm, pgd_t *pgdp, pud_t *pudp)
{
	__pgd_populate(pgdp, __pa(pudp), PUD_TYPE_TABLE);
}
#else
static inline void __pgd_populate(pgd_t *pgdp, phys_addr_t pudp, pgdval_t prot)
{
	BUILD_BUG();
}
#endif	/* CONFIG_PGTABLE_LEVELS > 3 */

extern pgd_t *pgd_alloc(struct mm_struct *mm);
extern void pgd_free(struct mm_struct *mm, pgd_t *pgdp);

static inline void __pmd_populate(pmd_t *pmdp, phys_addr_t ptep,
				  pmdval_t prot)
{
	set_pmd(pmdp, __pmd(__phys_to_pmd_val(ptep) | prot));
}

/*
 * Populate the pmdp entry with a pointer to the pte.  This pmd is part
 * of the mm address space.
 */
static inline void
pmd_populate_kernel(struct mm_struct *mm, pmd_t *pmdp, pte_t *ptep)
{
	/*
	 * The pmd must be loaded with the physical address of the PTE table
	 */
	__pmd_populate(pmdp, __pa(ptep), PMD_TYPE_TABLE);
}

static inline void
pmd_populate(struct mm_struct *mm, pmd_t *pmdp, pgtable_t ptep)
{
	__pmd_populate(pmdp, page_to_phys(ptep), PMD_TYPE_TABLE);
}
#define pmd_pgtable(pmd) pmd_page(pmd)

#endif
