import React, { useEffect, useState } from "react";
import { makeStyles } from "@material-ui/core/styles";
import { Card, CardContent, Grid, Icon, Typography } from "@material-ui/core";
import DirectionsRunIcon from "@material-ui/icons/DirectionsRun";
import DirectionsWalkIcon from "@material-ui/icons/DirectionsWalk";
import AirlineSeatReclineNormalIcon from "@material-ui/icons/AirlineSeatReclineNormal";
import HelpOutlineIcon from "@material-ui/icons/HelpOutline";

const useStyles = makeStyles({
  root: {
    borderRadius: 32,
  },
  cardContent: {
    padding: "5rem",
    maxWidth: "20rem",
  },
  profileImg: {
    borderRadius: "50em",
    width: "100%",
  },
  profileImgBox: {
    padding: "0.25em 0.75em",
    userSelect: "none",
  },
  profileName: {
    fontSize: "2.5rem",
    padding: "0.25rem 1.5rem",
    position: "relative",
    "& span.tilde": {
      position: "absolute",
      userSelect: "none",
      margin: "0",
    },
  },
  profileDescription: {
    padding: "0.25rem 0",
    fontSize: "1.5rem",
  },
  mainIcon: {
    fontSize: "6rem",
  },
});

// const labels = ["Walking", "Running", "Resting"];

// Server-Sent Events
// https://stackoverflow.com/a/12236019/9184658

function FitnessCard(props) {
  const classes = useStyles();

  const [pred, setPred] = useState({
    label: "(waiting...)",
    class_i: 3,
    confidence: 0.0,
  });

  const streamURL = "https://iotssc-307920.uc.r.appspot.com/stream"

  
  useEffect(() => {
    console.log("setting up event source")
    const eventSource = new EventSource(streamURL)

    eventSource.onopen = e => {
      console.log(e);
    }
    // eventSource.onmessage = e => {
    //   console.log('onmessage');
    //   console.log(e);
    // }
    eventSource.addEventListener('prediction', e => {
      console.log(e);
      setPred(JSON.parse(e.data));
    })
  
    // remove this handler
    // return () => (eventSource.removeEventListener('prediction'))
  }, []);

  const icons = [
    <DirectionsWalkIcon className={classes.mainIcon} />, // walking
    <DirectionsRunIcon className={classes.mainIcon} />, // running
    <AirlineSeatReclineNormalIcon className={classes.mainIcon} />, // resting
    <HelpOutlineIcon className={classes.mainIcon} />, // resting
  ];

  return (
    <Card className={classes.root}>
      <CardContent>
        <Grid
          container
          direction="column"
          alignContent="center"
          style={{ padding: "3rem 5rem" }}
        >
          <Grid item>{icons[pred.class_i]}</Grid>
          <Grid item>
            <Typography variant="h1" className={classes.profileName}>
              {pred.label}
            </Typography>
            <Typography
              variant="body1"
              className={classes.profileDescription}
              style={{ fontFamily: "monospace" }}
            >
              {(pred.confidence * 100).toFixed(2)}%
            </Typography>
          </Grid>
        </Grid>
      </CardContent>
    </Card>
  );
}

export default FitnessCard;
