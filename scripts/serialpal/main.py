from serialpal import SerialPal


def main():
    try:
        SerialPal().cmdloop()
    except KeyboardInterrupt:
        print("exiting...")
        SerialPal.cleanup()


if __name__ == "__main__":
    main()
