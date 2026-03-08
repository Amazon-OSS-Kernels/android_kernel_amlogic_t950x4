
ifeq ("$(SOC)","sc2")
MAILBOX=y
endif
ifeq ("$(SOC)","t5")
MAILBOX_PL=y
endif
ifeq ("$(SOC)","t5d")
MAILBOX_PL=y
endif

mailbox-$(MAILBOX) = mailbox.o mailbox-irq.o mailbox-htbl.o rpc-user.o
mailbox-$(MAILBOX_PL) = mailbox-pl.o mailbox-htbl.o rpc-user.o
